/*
# ==================================================================================
# Copyright 2024 Alexandre Huff.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==================================================================================
*/

#include <cstdio>
#include <memory>
#include <csignal>
#include <thread>
#include <string.h>
#include <sstream>
#include <bitset>

#include <ricxfcpp/xapp.hpp>
#include <ricxfcpp/message.hpp>
#include <rmr/RIC_message_types.h>
#include <mdclog/mdclog.h>

#include "a1mgr.hpp"
#include "control.hpp"
#include "utils.hpp"
#include "e2mgr.hpp"

extern "C" {
    #include "E2AP-PDU.h"
}


void a1_policy_handler(xapp::Message &msg, int mtype, int subid, int payload_len, xapp::Msg_component payload, void *usr_data ) {
    std::string response;
    bool error = false; // used to identify if we should reply to A1 Mediator with a successful or failure message

    mdclog_write(MDCLOG_DEBUG, "A1 Handler has been called and the policy is %s", payload.get());

    const char *xapp_id = (const char *)usr_data;

    std::string pmessage((const char *)payload.get(), payload_len); // payload might not be \0 terminated

    A1Manager a1(xapp_id);
    a1.parse_a1_policy(pmessage);

    mdclog_write(MDCLOG_INFO, "Received A1 Policy type=%s instance=%s", a1.getPolicyTypeId().c_str(), a1.getPolicyInstanceId().c_str());

    E2MgrCli e2mgr;
    if (e2mgr.fetch_connected_e2_nodes()) {

        for (auto &v : a1.getPolicyData()) {
            PLMN_Identity_t *plmnid = e2::utils::encodePlmnId(v.mcc.c_str(), v.mnc.c_str());
            std::string hex_plmnid;
            if (!e2::utils::decode_plmnid_to_hex(plmnid, hex_plmnid)) {
                mdclog_write(MDCLOG_ERR, "Unable to generate PLMN ID hex-String for mcc=%s and mnc=%s", v.mcc.c_str(), v.mnc.c_str());
                error = true;
                continue;
            }

            uint32_t nodeid = (uint32_t) std::strtoul(v.e2_node_id.c_str(), nullptr, 10);
            if (nodeid > ((1 << 29) - 1)) { // we use 29 bits to identify an E2 Node
                mdclog_write(MDCLOG_ERR, "E2 Node ID value of mcc=%s, mnc=%s in A1 Policy is higher than 29 bits", v.mcc.c_str(), v.mnc.c_str());
                error = true;
                continue;
            }
            std::string nodeid_bs = std::bitset<29>(nodeid).to_string();    // bit string
            std::string e2node = e2mgr.get_e2_node_inventory_name(hex_plmnid, nodeid_bs);
            if (e2node.empty()) {
                mdclog_write(MDCLOG_ERR, "E2 Node mcc=%s, mnc=%s, and e2_node_id=%s with plmnid=%s, and nbId=%s seems to be unavailable",
                    v.mcc.c_str(), v.mnc.c_str(), v.e2_node_id.c_str(), hex_plmnid.c_str(), nodeid_bs.c_str());
                error = true;
                continue;
            }

            for (auto &imsi : v.ues) {
                mdclog_write(MDCLOG_DEBUG, "Handing over UE=%s to E2Node mcc=%s, mnc=%s, e2nodeid=%s",
                    imsi.c_str(), v.mcc.c_str(), v.mnc.c_str(), v.e2_node_id.c_str());

                E2SM_RC_ControlHeader_t *header_fmt1 = generate_e2sm_rc_control_header_fmt1(imsi);
                if (!header_fmt1) {
                    mdclog_write(MDCLOG_ERR, "Unable to generate E2SM RC Control Header Format 1 from IMSI=%s", imsi.c_str());
                    error = true;
                    continue;
                }
                E2SM_RC_ControlMessage_t *msg_fmt1 = generate_e2sm_rc_control_message_fmt1(v.mcc, v.mnc, v.e2_node_id);
                if (!msg_fmt1) {
                    mdclog_write(MDCLOG_ERR, "Unable to generate E2SM RC Control Message Format 1 from mcc=%s, mnc=%s, e2_node_id=%s",
                        v.mcc.c_str(), v.mnc.c_str(), v.e2_node_id.c_str());
                    error = true;
                    continue;
                }
                RICcontrolHeader_t *ctrl_header = e2::utils::asn1_check_and_encode(&asn_DEF_E2SM_RC_ControlHeader, header_fmt1);
                if (!ctrl_header) {
                    mdclog_write(MDCLOG_ERR, "Unable to encode E2SM RC Control Header Format 1");
                    error = true;
                    continue;
                }
                // asn_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, msg_fmt1);   // FIXME remove
                RICcontrolMessage_t *ctrl_msg = e2::utils::asn1_check_and_encode(&asn_DEF_E2SM_RC_ControlMessage, msg_fmt1);
                if (!ctrl_msg) {
                    mdclog_write(MDCLOG_ERR, "Unable to encode E2SM RC Control Message Format 1");
                    error = true;
                    continue;
                }
                RANfunctionID_t func_id = 1;    // FIXME get this from the xapp descriptor file
                E2AP_PDU_t *pdu = generate_ric_control_request(func_id, ctrl_header, ctrl_msg);
                if (!pdu) {
                    mdclog_write(MDCLOG_ERR, "Unable to generate RIC Control Request");
                    error = true;
                    continue;
                }
                OCTET_STRING_t *data = e2::utils::asn1_check_and_encode(&asn_DEF_E2AP_PDU, pdu);
                if (!pdu) {
                    mdclog_write(MDCLOG_ERR, "Unable to encode RIC Control Request");
                    error = true;
                    continue;
                }

                mdclog_write(MDCLOG_INFO, "Sending Handover Request for UE=%s to E2Node mcc=%s, mnc=%s, e2nodeid=%s",
                    imsi.c_str(), v.mcc.c_str(), v.mnc.c_str(), v.e2_node_id.c_str());


                std::shared_ptr<unsigned char> meid((unsigned char *)strdup(e2node.c_str()));
                msg.Set_meid(meid);

                if (!msg.Send_msg(RIC_CONTROL_REQ, RMR_VOID_SUBID, (int)data->size, (unsigned char *)data->buf)) {
                    mdclog_write(MDCLOG_ERR, "Unable to send RIC Control Request to E2 Node mcc=%s, mnc=%s, e2_node_id=%s",
                        v.mcc.c_str(), v.mnc.c_str(), v.e2_node_id.c_str());
                    error = true;
                }

                ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, data);
                ASN_STRUCT_FREE(asn_DEF_E2AP_PDU, pdu);
                ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlHeader, header_fmt1);
                ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlMessage, msg_fmt1);
            }


        }

    } else {
        mdclog_write(MDCLOG_ERR, "Unable to request all connected E2 Nodes from E2MGR");
        error = true;
    }

    if (!error) {
        response = a1.serialize_a1_response(A1Status::OK);
    } else {
        response = a1.serialize_a1_response(A1Status::ERROR);
    }

    mdclog_write(MDCLOG_DEBUG, "A1 Response is %s\n", response.c_str());

    if (!msg.Send_msg(A1_POLICY_RESP, RMR_VOID_SUBID, response.length(), (unsigned char *)response.c_str())) {
        mdclog_write(MDCLOG_ERR, "Unable to send response for policy id %s to A1 Mediator", a1.getPolicyTypeId().c_str());
    }

}

void default_callback(xapp::Message &msg, int mtype, int subid, int payload_len, xapp::Msg_component payload, void *usr_data ) {
    mdclog_write(MDCLOG_WARN, "Unknown message type=%d, subid=%d, dropping message...", mtype, subid);
}

int main(int argc, char **argv) {
    // signal handler to stop gracefully
    int delivered_signal;
    sigset_t monitored_signals;
    sigfillset(&monitored_signals);
    sigprocmask(SIG_BLOCK, &monitored_signals, NULL);    // from now all new threads inherit this signal mask

    mdclog_format_initialize(1);

    mdclog_write(MDCLOG_INFO, "Starting handover-xapp");

    std::shared_ptr<Xapp> app = std::shared_ptr<Xapp>(new Xapp("4560", false));

    app->Add_msg_cb(A1_POLICY_REQ, a1_policy_handler, (void *) app->getXappId().c_str());
    app->Add_msg_cb(app->DEFAULT_CALLBACK, default_callback, NULL);

    std::thread listener(&Xapp::Run, app, 1);

    do {
        int ret_val = sigwait(&monitored_signals, &delivered_signal);   // we just wait for a signal to proceed
        if (ret_val == -1) {
            mdclog_write(MDCLOG_ERR, "sigwait failed");
        } else {
            switch (delivered_signal) {
                case SIGINT:
                    mdclog_write(MDCLOG_INFO, "SIGINT was received");
                    break;
                case SIGTERM:
                    mdclog_write(MDCLOG_INFO, "SIGTERM was received");
                    break;
                case SIGTRAP:
                    mdclog_write(MDCLOG_INFO, "SIGTRAP was received");
                    break;
                default:
                    mdclog_write(MDCLOG_WARN, "sigwait returned signal %d (%s). Ignored!", delivered_signal, strsignal(delivered_signal));
                    break;
            }
        }

    } while (delivered_signal != SIGTERM && delivered_signal != SIGINT && delivered_signal != SIGTRAP);

    mdclog_write(MDCLOG_INFO, "Shutting down xApp...");

    app->Halt();

    listener.join();

    mdclog_write(MDCLOG_INFO, "xApp has finished");

    return 0;
}

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

#include "control.hpp"
#include "utils.hpp"

#include <mdclog/mdclog.h>

extern "C" {
    #include "UEID-GNB.h"
    #include "PLMN-Identity.h"
    #include "E2SM-RC-ControlHeader-Format1.h"
    #include "E2SM-RC-ControlMessage-Format1.h"
    #include "E2SM-RC-ControlMessage-Format1-Item.h"
    #include "RANParameter-ValueType-Choice-Structure.h"
    #include "RANParameter-STRUCTURE.h"
    #include "RANParameter-STRUCTURE-Item.h"
    #include "RANParameter-ValueType-Choice-ElementFalse.h"
    #include "RANParameter-Value.h"
    #include "NR-CGI.h"
    #include "InitiatingMessage.h"
    #include "ProtocolIE-Field.h"
    #include "RANfunctionID.h"
}


E2SM_RC_ControlHeader_t *generate_e2sm_rc_control_header_fmt1(const std::string &imsi) {
    if (imsi.length() < 7 || imsi.length() > 15) {
        mdclog_write(MDCLOG_ERR, "IMSI size must have between 7 and 15 digits [0-9]");
        return nullptr;
    }

    std::string mcc = imsi.substr(0, 3);
    // TODO check how to dinamically figure out if MNC is 2 or 3 digits https://patents.google.com/patent/WO2008092821A2/en
    std::string mnc = imsi.substr(3, 3);    // for now we assume MNC size is always 3 digits
    std::string msin = imsi.substr(6, 9);

    long msin_number;
    try {
        msin_number = std::stol(msin);
    } catch (const std::exception &e) {
        mdclog_write(MDCLOG_ERR, "Unable to parse MSIN. Reason: %s", e.what());
        return nullptr;
    }

    E2SM_RC_ControlHeader_t *header = (E2SM_RC_ControlHeader_t *) calloc(1, sizeof(E2SM_RC_ControlHeader_t));
    header->ric_controlHeader_formats.present = E2SM_RC_ControlHeader__ric_controlHeader_formats_PR_controlHeader_Format1;
    header->ric_controlHeader_formats.choice.controlHeader_Format1 =
        (E2SM_RC_ControlHeader_Format1_t *) calloc(1, sizeof(E2SM_RC_ControlHeader_Format1_t));

    E2SM_RC_ControlHeader_Format1_t *header_fmt1 = header->ric_controlHeader_formats.choice.controlHeader_Format1;
    header_fmt1->ric_Style_Type = 3;
    header_fmt1->ric_ControlAction_ID = 1;
    header_fmt1->ueID.present = UEID_PR_gNB_UEID;
    header_fmt1->ueID.choice.gNB_UEID = (UEID_GNB_t *) calloc(1, sizeof(UEID_GNB_t));

    UEID_GNB_t *ueid_gnb = header_fmt1->ueID.choice.gNB_UEID;

    /**
     * AMF UE NGAP ID is an integer between 0..2^40-1
     * 2^40-1 = 1099511627775, which size is 13, but MCC+MNC+MSIN which identifies the UE requires size 16
     * Thus, we send the MSIN portion in AMF UE NGAP ID and MCC+MNC portion in GUAMI PLMN Identity
     *
     * MSIN has 9 digits, so we need 29 bits to send its max value which is 999.999.999
    */
    if (asn_long2INTEGER(&ueid_gnb->amf_UE_NGAP_ID, msin_number) != 0) {
        mdclog_write(MDCLOG_ERR, "Unable to encode MSIN to AMF_UE_NGAP_ID");
        return nullptr;
    }

    PLMN_Identity_t *plmnid = e2::utils::encodePlmnId(mcc.c_str(), mnc.c_str());
    ueid_gnb->guami.pLMNIdentity = *plmnid;
    if (plmnid) free(plmnid);

    ueid_gnb->guami.aMFRegionID.buf = (uint8_t *) calloc(1, sizeof(uint8_t)); // (8 bits)
    ueid_gnb->guami.aMFRegionID.buf[0] = (uint8_t) 128; // TODO this is a dummy value
    ueid_gnb->guami.aMFRegionID.size = 1;
    ueid_gnb->guami.aMFRegionID.bits_unused = 0;

    ueid_gnb->guami.aMFSetID.buf = (uint8_t *) calloc(2, sizeof(uint8_t)); // (10 bits)
    uint16_t v = (uint16_t) 4; // TODO this is a dummy vale (uint16_t is required to have room for 10 bits)
    v = v << 6; // we are only interested in 10 bits, so rotate them to the correct place
    ueid_gnb->guami.aMFSetID.buf[0] = (v >> 8); // only interested in the most significant bits (& 0x00ff only required for signed)
    ueid_gnb->guami.aMFSetID.buf[1] = v & 0x00ff; // we are only interested in the least significant bits
    ueid_gnb->guami.aMFSetID.size = 2;
    ueid_gnb->guami.aMFSetID.bits_unused = 6;

    ueid_gnb->guami.aMFPointer.buf = (uint8_t *) calloc(1, sizeof(uint8_t)); // (6 bits)
    ueid_gnb->guami.aMFPointer.buf[0] = (uint8_t) 1 << 2; // TODO this is a dummy value
    ueid_gnb->guami.aMFPointer.size = 1;
    ueid_gnb->guami.aMFPointer.bits_unused = 2;

    return header;
}

E2SM_RC_ControlMessage_t *generate_e2sm_rc_control_message_fmt1(const std::string &mcc, const std::string &mnc, const std::string &e2_node_id) {
    E2SM_RC_ControlMessage_t *msg  = (E2SM_RC_ControlMessage_t *) calloc(1, sizeof(E2SM_RC_ControlMessage_t));
    msg->ric_controlMessage_formats.present = E2SM_RC_ControlMessage__ric_controlMessage_formats_PR_controlMessage_Format1;
    msg->ric_controlMessage_formats.choice.controlMessage_Format1 =
        (E2SM_RC_ControlMessage_Format1_t *) calloc(1, sizeof(E2SM_RC_ControlMessage_Format1_t));

    E2SM_RC_ControlMessage_Format1_t *msg_fmt1 = msg->ric_controlMessage_formats.choice.controlMessage_Format1;
    E2SM_RC_ControlMessage_Format1_Item_t *param1 = (E2SM_RC_ControlMessage_Format1_Item_t *) calloc(1, sizeof(E2SM_RC_ControlMessage_Format1_Item_t));
    ASN_SEQUENCE_ADD(&msg_fmt1->ranP_List.list, param1);

    // Target Primary Cell ID
    param1->ranParameter_ID = 1;
    param1->ranParameter_valueType.present = RANParameter_ValueType_PR_ranP_Choice_Structure;
    param1->ranParameter_valueType.choice.ranP_Choice_Structure =
        (RANParameter_ValueType_Choice_Structure_t *) calloc(1, sizeof(RANParameter_ValueType_Choice_Structure_t));
    param1->ranParameter_valueType.choice.ranP_Choice_Structure->ranParameter_Structure =
        (RANParameter_STRUCTURE_t *) calloc(1, sizeof(RANParameter_STRUCTURE_t));
    param1->ranParameter_valueType.choice.ranP_Choice_Structure->ranParameter_Structure->sequence_of_ranParameters =
        (RANParameter_STRUCTURE_t::RANParameter_STRUCTURE__sequence_of_ranParameters *)
        calloc(1, sizeof(RANParameter_STRUCTURE_t::RANParameter_STRUCTURE__sequence_of_ranParameters));

    // CHOICE Target Cell
    RANParameter_STRUCTURE_Item_t *param2 = (RANParameter_STRUCTURE_Item_t *) calloc(1, sizeof(RANParameter_STRUCTURE_Item_t));
    ASN_SEQUENCE_ADD(&param1->ranParameter_valueType.choice.ranP_Choice_Structure->ranParameter_Structure->sequence_of_ranParameters->list, param2);

    param2->ranParameter_ID = 2;
    param2->ranParameter_valueType = (RANParameter_ValueType_t *) calloc(1, sizeof(RANParameter_ValueType_t));
    param2->ranParameter_valueType->present = RANParameter_ValueType_PR_ranP_Choice_Structure;
    param2->ranParameter_valueType->choice.ranP_Choice_Structure =
        (RANParameter_ValueType_Choice_Structure_t *) calloc(1, sizeof(RANParameter_ValueType_Choice_Structure_t));
    param2->ranParameter_valueType->choice.ranP_Choice_Structure->ranParameter_Structure =
        (RANParameter_STRUCTURE_t *) calloc(1, sizeof(RANParameter_STRUCTURE_t));
    param2->ranParameter_valueType->choice.ranP_Choice_Structure->ranParameter_Structure->sequence_of_ranParameters =
        (RANParameter_STRUCTURE_t::RANParameter_STRUCTURE__sequence_of_ranParameters *)
        calloc(1, sizeof(RANParameter_STRUCTURE_t::RANParameter_STRUCTURE__sequence_of_ranParameters));

    // NR Cell
    RANParameter_STRUCTURE_Item_t *param3 = (RANParameter_STRUCTURE_Item_t *) calloc(1, sizeof(RANParameter_STRUCTURE_Item_t));
    ASN_SEQUENCE_ADD(&param2->ranParameter_valueType->choice.ranP_Choice_Structure->ranParameter_Structure->sequence_of_ranParameters->list, param3);

    param3->ranParameter_ID = 3;
    param3->ranParameter_valueType = (RANParameter_ValueType_t *) calloc(1, sizeof(RANParameter_ValueType_t));
    param3->ranParameter_valueType->present = RANParameter_ValueType_PR_ranP_Choice_Structure;
    param3->ranParameter_valueType->choice.ranP_Choice_Structure =
        (RANParameter_ValueType_Choice_Structure_t *) calloc(1, sizeof(RANParameter_ValueType_Choice_Structure_t));
    param3->ranParameter_valueType->choice.ranP_Choice_Structure->ranParameter_Structure =
        (RANParameter_STRUCTURE_t *) calloc(1, sizeof(RANParameter_STRUCTURE_t));
    param3->ranParameter_valueType->choice.ranP_Choice_Structure->ranParameter_Structure->sequence_of_ranParameters =
        (RANParameter_STRUCTURE_t::RANParameter_STRUCTURE__sequence_of_ranParameters *)
        calloc(1, sizeof(RANParameter_STRUCTURE_t::RANParameter_STRUCTURE__sequence_of_ranParameters));

    // NR CGI
    RANParameter_STRUCTURE_Item_t *param4 = (RANParameter_STRUCTURE_Item_t *) calloc(1, sizeof(RANParameter_STRUCTURE_Item_t));
    ASN_SEQUENCE_ADD(&param3->ranParameter_valueType->choice.ranP_Choice_Structure->ranParameter_Structure->sequence_of_ranParameters->list, param4);

    param4->ranParameter_ID = 4;
    param4->ranParameter_valueType = (RANParameter_ValueType_t *) calloc(1, sizeof(RANParameter_ValueType_t));
    param4->ranParameter_valueType->present = RANParameter_ValueType_PR_ranP_Choice_ElementFalse;
    param4->ranParameter_valueType->choice.ranP_Choice_ElementFalse =
        (RANParameter_ValueType_Choice_ElementFalse_t *) calloc(1, sizeof(RANParameter_ValueType_Choice_ElementFalse_t));
    param4->ranParameter_valueType->choice.ranP_Choice_ElementFalse->ranParameter_value =
        (RANParameter_Value_t *) calloc(1, sizeof(RANParameter_Value_t));
    param4->ranParameter_valueType->choice.ranP_Choice_ElementFalse->ranParameter_value->present = RANParameter_Value_PR_valueOctS; // FIXME check if OctetString

    NR_CGI_t *nr_cgi = (NR_CGI_t *) calloc(1, sizeof(NR_CGI_t));
    PLMNIdentity_t *plmnid = e2::utils::encodePlmnId(mcc.c_str(), mnc.c_str());
    nr_cgi->pLMNIdentity = *plmnid;
    if (plmnid) free(plmnid);

    nr_cgi->nRCellIdentity.buf = (uint8_t *) calloc(5, sizeof(uint8_t)); // we need to allocate 40 bits to store 36 bits
    nr_cgi->nRCellIdentity.size = 5;
    nr_cgi->nRCellIdentity.bits_unused = 4; // 40 - 4 = 36 bits
    // we do not consider cell here, so the cell value is 0. Thus, NCI = gnbId * 2^(36-29) + cellid
    // we leave 7 bits for cellid
    unsigned long gnb_id = std::stoul(e2_node_id, nullptr, 10);
    uint64_t nci = gnb_id * 128;    // 36 - 29 = 7, and 2^7 is 128, and 128 + 0(cellid) = 128. Thus, nci = gnb_id * 128

    nci = nci << nr_cgi->nRCellIdentity.bits_unused;    // we need to put the first byte at the 40th position (36 + 4) in the bit string
    nr_cgi->nRCellIdentity.buf[0] = (nci >> 32) & 0xFF;
    nr_cgi->nRCellIdentity.buf[1] = (nci >> 24) & 0xFF;
    nr_cgi->nRCellIdentity.buf[2] = (nci >> 16) & 0xFF;
    nr_cgi->nRCellIdentity.buf[3] = (nci >> 8) & 0xFF;
    nr_cgi->nRCellIdentity.buf[4] = nci & 0xFF;
    // TODO check https://nrcalculator.firebaseapp.com/nrgnbidcalc.html
    // TODO check https://www.telecomhall.net/t/what-is-the-formula-for-cell-id-nci-in-5g-nr-networks/12623/2

    OCTET_STRING_t *nrcgi_data = e2::utils::asn1_check_and_encode(&asn_DEF_NR_CGI, nr_cgi);
    if (!nrcgi_data) {
        mdclog_write(MDCLOG_ERR, "Unable to encode NR CGI in E2SM RC Control Message Format 1");
        ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlHeader_Format1, msg);
        return nullptr;
    }

    param4->ranParameter_valueType->choice.ranP_Choice_ElementFalse->ranParameter_value->choice.valueOctS.size = nrcgi_data->size;
    param4->ranParameter_valueType->choice.ranP_Choice_ElementFalse->ranParameter_value->choice.valueOctS.buf = nrcgi_data->buf;
    free(nrcgi_data);

    return msg;
}

E2AP_PDU_t *generate_ric_control_request(RANfunctionID_t ran_func_id, RICcontrolHeader_t *header, RICcontrolMessage_t *msg) {
    E2AP_PDU_t *pdu = (E2AP_PDU_t *) calloc(1, sizeof(E2AP_PDU_t));
    pdu->present = E2AP_PDU_PR_initiatingMessage;
    pdu->choice.initiatingMessage = (InitiatingMessage_t *) calloc(1, sizeof(InitiatingMessage_t));
    pdu->choice.initiatingMessage->criticality = Criticality_reject;
    pdu->choice.initiatingMessage->procedureCode = ProcedureCode_id_RICcontrol;
    pdu->choice.initiatingMessage->value.present = InitiatingMessage__value_PR_RICcontrolRequest;
    RICcontrolRequest_t *ctrl_req = &pdu->choice.initiatingMessage->value.choice.RICcontrolRequest;

    // RIC Request ID
    RICcontrolRequest_IEs_t *req_id = (RICcontrolRequest_IEs_t *) calloc(1, sizeof(RICcontrolRequest_IEs_t));
    req_id->criticality = Criticality_reject;
    req_id->id = ProtocolIE_ID_id_RICrequestID;
    req_id->value.present = RICcontrolRequest_IEs__value_PR_RICrequestID;
    req_id->value.choice.RICrequestID.ricInstanceID = 1;    // hard coded for now
    req_id->value.choice.RICrequestID.ricRequestorID = 1;   // hard coded for now
    ASN_SEQUENCE_ADD(&ctrl_req->protocolIEs.list, req_id);

    // RAN Function ID
    RICcontrolRequest_IEs_t *func_id = (RICcontrolRequest_IEs_t *) calloc(1, sizeof(RICcontrolRequest_IEs_t));
    func_id->criticality = Criticality_reject;
    func_id->id = ProtocolIE_ID_id_RANfunctionID;
    func_id->value.present = RICcontrolRequest_IEs__value_PR_RANfunctionID;
    func_id->value.choice.RANfunctionID = ran_func_id;
    ASN_SEQUENCE_ADD(&ctrl_req->protocolIEs.list, func_id);

    // RIC Control Header
    RICcontrolRequest_IEs_t *ctrl_header = (RICcontrolRequest_IEs_t *) calloc(1, sizeof(RICcontrolRequest_IEs_t));
    ctrl_header->criticality = Criticality_reject;
    ctrl_header->id = ProtocolIE_ID_id_RICcontrolHeader;
    ctrl_header->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolHeader;
    ctrl_header->value.choice.RICcontrolHeader = *header;
    ASN_SEQUENCE_ADD(&ctrl_req->protocolIEs.list, ctrl_header);

    // RIC Control Message
    RICcontrolRequest_IEs_t *ctrl_msg = (RICcontrolRequest_IEs_t *) calloc(1, sizeof(RICcontrolRequest_IEs_t));
    ctrl_msg->criticality = Criticality_reject;
    ctrl_msg->id = ProtocolIE_ID_id_RICcontrolMessage;
    ctrl_msg->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolMessage;
    ctrl_msg->value.choice.RICcontrolHeader = *msg;
    ASN_SEQUENCE_ADD(&ctrl_req->protocolIEs.list, ctrl_msg);

    return pdu;
}

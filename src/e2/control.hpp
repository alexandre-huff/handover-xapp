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

#ifndef E2_CONTROL_HPP
#define E2_CONTROL_HPP

#include <string>

extern "C" {
    #include "E2AP-PDU.h"
    #include "E2SM-RC-ControlHeader.h"
    #include "E2SM-RC-ControlMessage.h"
    #include "RANfunctionID.h"
    #include "RICcontrolHeader.h"
    #include "RICcontrolMessage.h"
}

E2SM_RC_ControlHeader_t *generate_e2sm_rc_control_header_fmt1(const std::string &imsi);
E2SM_RC_ControlMessage_t *generate_e2sm_rc_control_message_fmt1(const std::string &mcc, const std::string &mnc, const std::string &e2_node_id);
E2AP_PDU_t *generate_ric_control_request(RANfunctionID_t func_id, RICcontrolHeader_t *header, RICcontrolMessage_t *msg);


#endif

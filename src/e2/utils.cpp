/*****************************************************************************
#                                                                            *
# Copyright 2024 Alexandre Huff                                              *
#                                                                            *
# Licensed under the Apache License, Version 2.0 (the "License");            *
# you may not use this file except in compliance with the License.           *
# You may obtain a copy of the License at                                    *
#                                                                            *
#      http://www.apache.org/licenses/LICENSE-2.0                            *
#                                                                            *
# Unless required by applicable law or agreed to in writing, software        *
# distributed under the License is distributed on an "AS IS" BASIS,          *
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
# See the License for the specific language governing permissions and        *
# limitations under the License.                                             *
#                                                                            *
******************************************************************************/

#include "utils.hpp"

#include <string>
#include <stdexcept>
#include <mdclog/mdclog.h>

/*
    This function checks and encodes a given asn1 type descriptor.
    Returns NULL in case of error.
    It is the caller responsibility to free the returned struct pointer.
*/
OCTET_STRING_t *e2::utils::asn1_check_and_encode(const asn_TYPE_descriptor_s *type_to_encode, const void *structure_to_encode) {
    const int BUF_SIZE = 8192;

    OCTET_STRING_t *encoded_ostr = NULL;

    char error_buf[300] = {0, };
    size_t errlen = 300;
    int ret = asn_check_constraints(type_to_encode, structure_to_encode, error_buf, &errlen);
    if (ret == 0) { // constraints are met
        uint8_t e2smbuffer[BUF_SIZE] = {0, };
        size_t e2smbuffer_size = BUF_SIZE;

        asn_codec_ctx_t *opt_cod = NULL;
        asn_enc_rval_t er = asn_encode_to_buffer(opt_cod,
                                ATS_ALIGNED_BASIC_PER,
                                type_to_encode, structure_to_encode,
                                e2smbuffer, e2smbuffer_size);

        if (er.encoded == -1) {
            mdclog_write(MDCLOG_ERR, "Failed to encode %s", er.failed_type->name);

        } else if (er.encoded > BUF_SIZE) {
           mdclog_write(MDCLOG_ERR, "Size of encoded data is greater than buffer size: %ld > %d", er.encoded, BUF_SIZE);

        } else {
            encoded_ostr = OCTET_STRING_new_fromBuf(type_to_encode, (const char *)e2smbuffer, er.encoded);
            if (encoded_ostr == NULL) {
                mdclog_write(MDCLOG_ERR, "Unable to allocate new OCTET_STRING for %s", type_to_encode->name);
            }
        }

    } else {
        mdclog_write(MDCLOG_ERR, "Check constraints failed for %s: error length = %lu, error buf = %s", type_to_encode->name, errlen, error_buf);
    }

    return encoded_ostr;
}

/*
    This function decodes and checks a given asn1 type descriptor.
    It is the caller responsibility to free the returned struct pointer in case of success.
*/
bool e2::utils::asn1_decode_and_check(const asn_TYPE_descriptor_s *type_to_decode, void **structure_to_decode, const uint8_t *buffer, size_t bufsize) {
    asn_dec_rval_t dval = asn_decode(nullptr, ATS_ALIGNED_BASIC_PER, type_to_decode, structure_to_decode, buffer, bufsize);
    if (dval.code != RC_OK) {
        mdclog_write(MDCLOG_ERR, "Failed to decode %s. Length of data = %lu, return code = %d", type_to_decode->name, dval.consumed, dval.code);
        return false;
    }

    mdclog_write(MDCLOG_ERR, "%s length of data = %lu, return code = %d", type_to_decode->name, dval.consumed, dval.code);

    char error_buf[300] = {0, };
    size_t errlen = 300;
    int ret = asn_check_constraints(type_to_decode, *structure_to_decode, error_buf, &errlen);

    if (ret != 0) {
        mdclog_write(MDCLOG_ERR, "Check constraints failed for %s: error length = %lu, error buf = %s", type_to_decode->name, errlen, error_buf);
        ASN_STRUCT_FREE(*type_to_decode, *structure_to_decode);
        return false;
    }

    if (mdclog_level_get() == MDCLOG_DEBUG) {
        asn_fprint(stderr, type_to_decode, *structure_to_decode);
    }

    return true;
}

/*
    Encodes MCC and MNC to PLMN ID

    410 32 becomes 14 F0 23
    or
    410 532 becomes 14 50 23
*/
PLMN_Identity_t *e2::utils::encodePlmnId(const char *mcc, const char *mnc) {
    PLMN_Identity_t *plmn = (PLMN_Identity_t *)calloc(1, sizeof(PLMN_Identity_t));
    plmn->size = 3;  // the size according to E2AP specification
    plmn->buf = (uint8_t *)calloc(3, sizeof(uint8_t));

    plmn->buf[0] = ((int)(mcc[1])) << 4;
    plmn->buf[0] |= ((int)(mcc[0])) & 0x0F;

    if (strlen((char *)mnc) == 3) {
        plmn->buf[1] = ((int)mnc[0]) << 4;
        plmn->buf[1] |= ((int)mcc[2]) & 0x0F;
        plmn->buf[2] = ((int)mnc[2]) << 4;
        plmn->buf[2] |= ((int)mnc[1]) & 0x0F;

    } else {
        plmn->buf[1] = 0xF0;
        plmn->buf[1] |= ((int)mcc[2]) & 0x0F;
        plmn->buf[2] = ((int)mnc[1]) << 4;
        plmn->buf[2] |= ((int)mnc[0]) & 0x0F;
    }

    mdclog_write(MDCLOG_DEBUG, "PLMN Identity encoded for mcc=%s mnc=%s", mcc, mnc);

    if (mdclog_level_get() == MDCLOG_DEBUG) {
        xer_fprint(stderr, &asn_DEF_PLMN_Identity, plmn);
    }

    return plmn;
}

/*
    Decodes PLMN ID to MCC and MNC

    14 F0 23 becomes 410 32
    or
    14 50 23 becomes 410 532
*/
bool e2::utils::decodePlmnId(PLMN_Identity_t *plmnid, std::string &mcc, std::string &mnc) {
    if (!plmnid || plmnid->size != 3) {
        return false;
    }
    mcc = std::to_string((int)((plmnid->buf[0] & 0x0F)));
    mcc += std::to_string((int)((plmnid->buf[0] >> 4) & 0x0F));
    mcc += std::to_string((int)(plmnid->buf[1] & 0x0F));

    if (((plmnid->buf[1] >> 4) & 0x0F) == 15) { // means F
        mnc = std::to_string((int)(plmnid->buf[2] & 0x0F));
        mnc += std::to_string((int)((plmnid->buf[2] >> 4) & 0x0F));
    } else {
        mnc = std::to_string((int)((plmnid->buf[1] >> 4) & 0x0F));
        mnc += std::to_string((int)(plmnid->buf[2] & 0x0F));
        mnc += std::to_string((int)((plmnid->buf[2] >> 4) & 0x0F));
    }

    return true;
}

/*
	Decodes the PLMN ID to its Hex-String representation
*/
bool e2::utils::decode_plmnid_to_hex(const PLMN_Identity_t *plmnid, std::string &hex_plmnid) {
    if (!plmnid || plmnid->size != 3) {
        return false;
    }

    hex_plmnid = std::to_string( (plmnid->buf[0] >> 4) & 0x0F );
    hex_plmnid += std::to_string( plmnid->buf[0] & 0x0F );
    hex_plmnid += std::to_string( (plmnid->buf[1] >> 4) & 0x0F );
    hex_plmnid += std::to_string( plmnid->buf[1] & 0x0F );
    hex_plmnid += std::to_string( (plmnid->buf[2] >> 4) & 0x0F );
    hex_plmnid += std::to_string( plmnid->buf[2] & 0x0F );

    return true;
}

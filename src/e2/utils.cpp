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

NR_CGI_t *e2::utils::encode_NR_CGI(const std::string &mcc, const std::string &mnc, const uint32_t gnb_id, const uint16_t pci) {
    if (gnb_id > ((1 << GNB_ID_LENGTH) - 1)) {
        mdclog_write(MDCLOG_ERR, "gNB ID value cannot be higher than %d bits to encode NR CGI", GNB_ID_LENGTH);
        return nullptr;
    }

    if (pci > (1 << (36 - GNB_ID_LENGTH) - 1)) {
        mdclog_write(MDCLOG_ERR, "PCI value cannot be higher than %d bits to encode NR CGI", 36 - GNB_ID_LENGTH);
        return nullptr;
    }

    PLMNIdentity_t *plmnid = encodePlmnId(mcc.c_str(), mnc.c_str()); // validates mcc and mnc
    if (!plmnid) {
        mdclog_write(MDCLOG_ERR, "Unable to encode PLMN Identity for NR CGI");
        return nullptr;
    }

    NR_CGI_t *nr_cgi = (NR_CGI_t *) calloc(1, sizeof(NR_CGI_t));
    nr_cgi->pLMNIdentity = *plmnid;
    if (plmnid) free(plmnid);

    // NCI
    nr_cgi->nRCellIdentity.buf = (uint8_t *) calloc(5, sizeof(uint8_t)); // we need to allocate 40 bits to store 36 bits
    nr_cgi->nRCellIdentity.size = 5;
    nr_cgi->nRCellIdentity.bits_unused = 4; // 40 - 4 = 36 bits
    // Taking into account that GNB_ID Length is 29 bits, we have the following formula: NCI = gnbId * 2^(36-29) + cellid
    // we leave 7 bits for cellid (pci) as long as GNB_ID length is 29 bits
    // uint64_t nci = gnb_id << (36 - GNB_ID_LENGTH) + pci;
    uint64_t nci = (gnb_id << (36 - GNB_ID_LENGTH)) | pci;
    // Source: 3GPP TS 38.413, Sections 9.3.1.6 and 9.3.1.7
    // Source: https://nrcalculator.firebaseapp.com/nrgnbidcalc.html
    // Source: https://www.telecomhall.net/t/what-is-the-formula-for-cell-id-nci-in-5g-nr-networks/12623/2
    // Source: https://nrcalculator.web.app/nrgnbidcalc_tw.html

    nci = nci << nr_cgi->nRCellIdentity.bits_unused;    // we need to put the first byte at the 40th position (36 + 4) in the bit string
    nr_cgi->nRCellIdentity.buf[0] = (nci >> 32) & 0xFF;
    nr_cgi->nRCellIdentity.buf[1] = (nci >> 24) & 0xFF;
    nr_cgi->nRCellIdentity.buf[2] = (nci >> 16) & 0xFF;
    nr_cgi->nRCellIdentity.buf[3] = (nci >> 8) & 0xFF;
    nr_cgi->nRCellIdentity.buf[4] = nci & 0xFF;

    if (mdclog_level_get() >= MDCLOG_DEBUG) {
        asn_fprint(stderr, &asn_DEF_NR_CGI, nr_cgi);
    }

    return nr_cgi;
}

bool e2::utils::decode_NR_CGI(const NR_CGI_t *nr_cgi, std::string &mcc, std::string &mnc, uint32_t &gnb_id, uint16_t &pci) {
    if (!nr_cgi) {
        mdclog_write(MDCLOG_ERR, "Unable to decode NR CGI, nil?");
        return false;
    }

    // ASN1 has different names for PLMN Identity types in E2AP and E2SM, so we have to typecast
    if (decodePlmnId((PLMN_Identity_t *)&nr_cgi->pLMNIdentity, mcc, mnc)) {
        mdclog_write(MDCLOG_ERR, "Unable to decode PLMN Identity from NR CGI");
        return false;
    }

    // Source: 3GPP TS 38.413, Sections 9.3.1.6 and 9.3.1.7
    uint64_t nci;   // NR Cell Idendity
    nci = (uint64_t)nr_cgi->nRCellIdentity.buf[0] << 32;
    nci |= (uint64_t)nr_cgi->nRCellIdentity.buf[1] << 24;
    nci |= (uint64_t)nr_cgi->nRCellIdentity.buf[2] << 16;
    nci |= (uint64_t)nr_cgi->nRCellIdentity.buf[3] << 8;
    nci |= (uint64_t)nr_cgi->nRCellIdentity.buf[4];

    // bit rotation based on 3GPP TS 38.413, Sections 9.3.1.6 and 9.3.1.7
    nci = nci >> nr_cgi->nRCellIdentity.bits_unused;    // first, we have to rotate unused bits to the right
    pci = nci & (36 - GNB_ID_LENGTH);                   // extract only the remaining bits from cell id
    gnb_id = nci >> (36 - GNB_ID_LENGTH);

    if (gnb_id > ((1 << GNB_ID_LENGTH) - 1)) {
        mdclog_write(MDCLOG_ERR, "Unable to decode Cell ID from NR CGI, value cannot be higher than %d bits", GNB_ID_LENGTH);
        return false;
    }

    mdclog_write(MDCLOG_DEBUG, "NR CGI decoded to MCC=%s, MNC=%s, gNB_ID=%u, Cell_ID=%u", mcc.c_str(), mnc.c_str(), gnb_id, pci);

    return true;
}

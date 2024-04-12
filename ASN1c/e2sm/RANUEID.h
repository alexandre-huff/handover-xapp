/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-COMMON-IEs"
 * 	found in "E2SM-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_RANUEID_H_
#define	_RANUEID_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RANUEID */
typedef OCTET_STRING_t	 RANUEID_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_RANUEID_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_RANUEID;
asn_struct_free_f RANUEID_free;
asn_struct_print_f RANUEID_print;
asn_constr_check_f RANUEID_constraint;
ber_type_decoder_f RANUEID_decode_ber;
der_type_encoder_f RANUEID_encode_der;
xer_type_decoder_f RANUEID_decode_xer;
xer_type_encoder_f RANUEID_encode_xer;
per_type_decoder_f RANUEID_decode_uper;
per_type_encoder_f RANUEID_encode_uper;
per_type_decoder_f RANUEID_decode_aper;
per_type_encoder_f RANUEID_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _RANUEID_H_ */
#include "asn_internal.h"

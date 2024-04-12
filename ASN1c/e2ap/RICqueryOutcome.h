/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-IEs"
 * 	found in "E2AP-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_RICqueryOutcome_H_
#define	_RICqueryOutcome_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RICqueryOutcome */
typedef OCTET_STRING_t	 RICqueryOutcome_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RICqueryOutcome;
asn_struct_free_f RICqueryOutcome_free;
asn_struct_print_f RICqueryOutcome_print;
asn_constr_check_f RICqueryOutcome_constraint;
ber_type_decoder_f RICqueryOutcome_decode_ber;
der_type_encoder_f RICqueryOutcome_encode_der;
xer_type_decoder_f RICqueryOutcome_decode_xer;
xer_type_encoder_f RICqueryOutcome_encode_xer;
per_type_decoder_f RICqueryOutcome_decode_uper;
per_type_encoder_f RICqueryOutcome_encode_uper;
per_type_decoder_f RICqueryOutcome_decode_aper;
per_type_encoder_f RICqueryOutcome_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _RICqueryOutcome_H_ */
#include "asn_internal.h"
/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-RC-IEs"
 * 	found in "E2SM-RC-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_ListOfAdditionalSupportedFormats_UEGroupControl_H_
#define	_ListOfAdditionalSupportedFormats_UEGroupControl_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct AdditionalSupportedFormat_UEGroupControl;

/* ListOfAdditionalSupportedFormats-UEGroupControl */
typedef struct ListOfAdditionalSupportedFormats_UEGroupControl {
	A_SEQUENCE_OF(struct AdditionalSupportedFormat_UEGroupControl) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ListOfAdditionalSupportedFormats_UEGroupControl_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ListOfAdditionalSupportedFormats_UEGroupControl;
extern asn_SET_OF_specifics_t asn_SPC_ListOfAdditionalSupportedFormats_UEGroupControl_specs_1;
extern asn_TYPE_member_t asn_MBR_ListOfAdditionalSupportedFormats_UEGroupControl_1[1];
extern asn_per_constraints_t asn_PER_type_ListOfAdditionalSupportedFormats_UEGroupControl_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _ListOfAdditionalSupportedFormats_UEGroupControl_H_ */
#include "asn_internal.h"

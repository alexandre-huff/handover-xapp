/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-RC-IEs"
 * 	found in "E2SM-RC-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_E2SM_RC_QueryOutcome_Format2_H_
#define	_E2SM_RC_QueryOutcome_Format2_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct E2SM_RC_QueryOutcome_Format2_ItemUE;

/* E2SM-RC-QueryOutcome-Format2 */
typedef struct E2SM_RC_QueryOutcome_Format2 {
	struct E2SM_RC_QueryOutcome_Format2__ueInfo_List {
		A_SEQUENCE_OF(struct E2SM_RC_QueryOutcome_Format2_ItemUE) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} ueInfo_List;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} E2SM_RC_QueryOutcome_Format2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_E2SM_RC_QueryOutcome_Format2;
extern asn_SEQUENCE_specifics_t asn_SPC_E2SM_RC_QueryOutcome_Format2_specs_1;
extern asn_TYPE_member_t asn_MBR_E2SM_RC_QueryOutcome_Format2_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _E2SM_RC_QueryOutcome_Format2_H_ */
#include "asn_internal.h"

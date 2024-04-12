/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-RC-IEs"
 * 	found in "E2SM-RC-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_E2SM_RC_QueryHeader_Format1_H_
#define	_E2SM_RC_QueryHeader_Format1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "RIC-Style-Type.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct RANParameter_Testing;
struct Associated_UE_Info;

/* E2SM-RC-QueryHeader-Format1 */
typedef struct E2SM_RC_QueryHeader_Format1 {
	RIC_Style_Type_t	 ric_Style_Type;
	struct RANParameter_Testing	*associatedE2NodeInfo;	/* OPTIONAL */
	struct Associated_UE_Info	*associatedUEInfo;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} E2SM_RC_QueryHeader_Format1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_E2SM_RC_QueryHeader_Format1;
extern asn_SEQUENCE_specifics_t asn_SPC_E2SM_RC_QueryHeader_Format1_specs_1;
extern asn_TYPE_member_t asn_MBR_E2SM_RC_QueryHeader_Format1_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _E2SM_RC_QueryHeader_Format1_H_ */
#include "asn_internal.h"

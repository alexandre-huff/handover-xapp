/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-COMMON-IEs"
 * 	found in "E2SM-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_UEID_EN_GNB_H_
#define	_UEID_EN_GNB_H_


#include "asn_application.h"

/* Including external dependencies */
#include "ENB-UE-X2AP-ID.h"
#include "ENB-UE-X2AP-ID-Extension.h"
#include "GlobalENB-ID.h"
#include "GNB-CU-UE-F1AP-ID.h"
#include "RANUEID.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct UEID_GNB_CU_CP_E1AP_ID_List;

/* UEID-EN-GNB */
typedef struct UEID_EN_GNB {
	ENB_UE_X2AP_ID_t	 m_eNB_UE_X2AP_ID;
	ENB_UE_X2AP_ID_Extension_t	*m_eNB_UE_X2AP_ID_Extension;	/* OPTIONAL */
	GlobalENB_ID_t	 globalENB_ID;
	GNB_CU_UE_F1AP_ID_t	*gNB_CU_UE_F1AP_ID;	/* OPTIONAL */
	struct UEID_GNB_CU_CP_E1AP_ID_List	*gNB_CU_CP_UE_E1AP_ID_List;	/* OPTIONAL */
	RANUEID_t	*ran_UEID;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UEID_EN_GNB_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UEID_EN_GNB;
extern asn_SEQUENCE_specifics_t asn_SPC_UEID_EN_GNB_specs_1;
extern asn_TYPE_member_t asn_MBR_UEID_EN_GNB_1[6];

#ifdef __cplusplus
}
#endif

#endif	/* _UEID_EN_GNB_H_ */
#include "asn_internal.h"

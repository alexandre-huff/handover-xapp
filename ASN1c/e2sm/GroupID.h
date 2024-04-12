/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-COMMON-IEs"
 * 	found in "E2SM-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_GroupID_H_
#define	_GroupID_H_


#include "asn_application.h"

/* Including external dependencies */
#include "FiveQI.h"
#include "QCI.h"
#include "constr_CHOICE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum GroupID_PR {
	GroupID_PR_NOTHING,	/* No components present */
	GroupID_PR_fiveGC,
	GroupID_PR_ePC
	/* Extensions may appear below */
	
} GroupID_PR;

/* GroupID */
typedef struct GroupID {
	GroupID_PR present;
	union GroupID_u {
		FiveQI_t	 fiveGC;
		QCI_t	 ePC;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GroupID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GroupID;

#ifdef __cplusplus
}
#endif

#endif	/* _GroupID_H_ */
#include "asn_internal.h"

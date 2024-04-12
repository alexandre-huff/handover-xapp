/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-RC-IEs"
 * 	found in "E2SM-RC-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#ifndef	_EventTrigger_UE_Info_Item_H_
#define	_EventTrigger_UE_Info_Item_H_


#include "asn_application.h"

/* Including external dependencies */
#include "RIC-EventTrigger-UE-ID.h"
#include "LogicalOR.h"
#include "constr_CHOICE.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum EventTrigger_UE_Info_Item__ueType_PR {
	EventTrigger_UE_Info_Item__ueType_PR_NOTHING,	/* No components present */
	EventTrigger_UE_Info_Item__ueType_PR_ueType_Choice_Individual,
	EventTrigger_UE_Info_Item__ueType_PR_ueType_Choice_Group
	/* Extensions may appear below */
	
} EventTrigger_UE_Info_Item__ueType_PR;

/* Forward declarations */
struct EventTrigger_UE_Info_Item_Choice_Individual;
struct EventTrigger_UE_Info_Item_Choice_Group;

/* EventTrigger-UE-Info-Item */
typedef struct EventTrigger_UE_Info_Item {
	RIC_EventTrigger_UE_ID_t	 eventTriggerUEID;
	struct EventTrigger_UE_Info_Item__ueType {
		EventTrigger_UE_Info_Item__ueType_PR present;
		union EventTrigger_UE_Info_Item__ueType_u {
			struct EventTrigger_UE_Info_Item_Choice_Individual	*ueType_Choice_Individual;
			struct EventTrigger_UE_Info_Item_Choice_Group	*ueType_Choice_Group;
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} ueType;
	LogicalOR_t	*logicalOR;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EventTrigger_UE_Info_Item_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EventTrigger_UE_Info_Item;
extern asn_SEQUENCE_specifics_t asn_SPC_EventTrigger_UE_Info_Item_specs_1;
extern asn_TYPE_member_t asn_MBR_EventTrigger_UE_Info_Item_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _EventTrigger_UE_Info_Item_H_ */
#include "asn_internal.h"

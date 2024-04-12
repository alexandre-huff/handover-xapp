/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-RC-IEs"
 * 	found in "E2SM-RC-R003-v03.00.asn1"
 * 	`asn1c -fcompound-names -findirect-choice -fincludes-quoted -fno-include-deps -gen-PER -no-gen-OER -no-gen-example`
 */

#include "E2SM-RC-QueryOutcome.h"

#include "E2SM-RC-QueryOutcome-Format1.h"
#include "E2SM-RC-QueryOutcome-Format2.h"
static asn_per_constraints_t asn_PER_type_ric_queryOutcome_formats_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  1,  1,  0,  1 }	/* (0..1,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_ric_queryOutcome_formats_2[] = {
	{ ATF_POINTER, 0, offsetof(struct E2SM_RC_QueryOutcome__ric_queryOutcome_formats, choice.queryOutcome_Format1),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_E2SM_RC_QueryOutcome_Format1,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"queryOutcome-Format1"
		},
	{ ATF_POINTER, 0, offsetof(struct E2SM_RC_QueryOutcome__ric_queryOutcome_formats, choice.queryOutcome_Format2),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_E2SM_RC_QueryOutcome_Format2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"queryOutcome-Format2"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_ric_queryOutcome_formats_tag2el_2[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* queryOutcome-Format1 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* queryOutcome-Format2 */
};
static asn_CHOICE_specifics_t asn_SPC_ric_queryOutcome_formats_specs_2 = {
	sizeof(struct E2SM_RC_QueryOutcome__ric_queryOutcome_formats),
	offsetof(struct E2SM_RC_QueryOutcome__ric_queryOutcome_formats, _asn_ctx),
	offsetof(struct E2SM_RC_QueryOutcome__ric_queryOutcome_formats, present),
	sizeof(((struct E2SM_RC_QueryOutcome__ric_queryOutcome_formats *)0)->present),
	asn_MAP_ric_queryOutcome_formats_tag2el_2,
	2,	/* Count of tags in the map */
	0, 0,
	2	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ric_queryOutcome_formats_2 = {
	"ric-queryOutcome-formats",
	"ric-queryOutcome-formats",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ 0, &asn_PER_type_ric_queryOutcome_formats_constr_2, CHOICE_constraint },
	asn_MBR_ric_queryOutcome_formats_2,
	2,	/* Elements count */
	&asn_SPC_ric_queryOutcome_formats_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_E2SM_RC_QueryOutcome_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct E2SM_RC_QueryOutcome, ric_queryOutcome_formats),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_ric_queryOutcome_formats_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ric-queryOutcome-formats"
		},
};
static const ber_tlv_tag_t asn_DEF_E2SM_RC_QueryOutcome_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_E2SM_RC_QueryOutcome_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* ric-queryOutcome-formats */
};
static asn_SEQUENCE_specifics_t asn_SPC_E2SM_RC_QueryOutcome_specs_1 = {
	sizeof(struct E2SM_RC_QueryOutcome),
	offsetof(struct E2SM_RC_QueryOutcome, _asn_ctx),
	asn_MAP_E2SM_RC_QueryOutcome_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_E2SM_RC_QueryOutcome = {
	"E2SM-RC-QueryOutcome",
	"E2SM-RC-QueryOutcome",
	&asn_OP_SEQUENCE,
	asn_DEF_E2SM_RC_QueryOutcome_tags_1,
	sizeof(asn_DEF_E2SM_RC_QueryOutcome_tags_1)
		/sizeof(asn_DEF_E2SM_RC_QueryOutcome_tags_1[0]), /* 1 */
	asn_DEF_E2SM_RC_QueryOutcome_tags_1,	/* Same as above */
	sizeof(asn_DEF_E2SM_RC_QueryOutcome_tags_1)
		/sizeof(asn_DEF_E2SM_RC_QueryOutcome_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_E2SM_RC_QueryOutcome_1,
	1,	/* Elements count */
	&asn_SPC_E2SM_RC_QueryOutcome_specs_1	/* Additional specs */
};


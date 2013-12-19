/*
 * Copyright (C) 1992,1993 NEC Corporation.
 * $Id: macro.h,v 2.6 1994/04/19 10:16:49 uchida Exp $ (NEC)
 */
#define	MACRO_MAXARG	10

#define	M_DOC_BEGIN	0
#define	M_DOC_END	1
#define	M_PLAIN_BEGIN	2
#define	M_PLAIN_END	3
#define	M_EXAM_BEGIN	4
#define	M_EXAM_END	5
#define	M_SET_SEC	6
#define	M_APPENDIX	7
#define	M_APDX_BEGIN	8
#define	M_BLANK		9
#define	M_PAGE		10
#define	M_NEWLINE	11
#define	M_CENTER_BEGIN	12
#define	M_CENTER_END	13
#define	M_RIGHT_BEGIN	14
#define	M_RIGHT_END	15
#define	M_INDENT	16
#define	M_INDENT0	17
#define	M_TITLE		18
#define	M_FOOTN_BEGIN	19
#define	M_FOOTN_END	20
#define	M_REFER_BEGIN	21
#define	M_REFER_END	22
#define	M_BOLD_BEGIN	23
#define	M_BOLD_END	24
#define	M_INDEX_BEGIN	25
#define	M_INDEX_END	26
#define	M_SECTION_END	27
#define	M_SECTION_1	28
#define	M_SECTION_2	(M_SECTION_1 + 1)
#define	M_SECTION_3	(M_SECTION_1 + 2)
#define	M_SECTION_4	(M_SECTION_1 + 3)
#define	M_SECTION_5	(M_SECTION_1 + 4)
#define	M_SECTION	(M_SECTION_1 + 5)
#define	M_SETSEC_1	(M_SECTION + 1)
#define	M_SETSEC_2	(M_SETSEC_1 + 1)
#define	M_SETSEC_3	(M_SETSEC_1 + 2)
#define	M_SETSEC_4	(M_SETSEC_1 + 3)
#define	M_SETSEC_5	(M_SETSEC_1 + 4)
#define	M_SETSEC	(M_SETSEC_1 + 5)

#define	M_MAXNUM	(M_SETSEC + 1)

/*
 * Definitions for Macfro Name, ID Number, Number of Arguments
 */
struct	macNames {
	char	*mname_name;
	int	mname_number;
	char	*mname_argattr;
};

/*
 * Compiled Macros
 */
struct	cmpMac {
	struct	cmpMac *cmac_next;
	int 	cmac_argnum;
	union {
		char   	mic_argtype;
		char	*mic_str;
	} cmac_content;
#define	cmac_argtype	cmac_content.mic_argtype
#define	cmac_str	cmac_content.mic_str
};

struct	macDefs {
	int	mdef_number;
	char	*mdef_def;
};

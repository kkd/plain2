/*
 * Copyright (C) 1992,1993 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: macro.c,v 2.10 1994/04/19 10:16:48 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include "plain2.h"
#include "macro.h"

#define	MACRO_SUFFIX	".p2"

struct	macNames macNames[] = {
	"DOC_BEGIN",	M_DOC_BEGIN,	"#$$",
	"DOC_END",	M_DOC_END,	"",
	"PLAIN_BEGIN",	M_PLAIN_BEGIN,	"",
	"PLAIN_END",	M_PLAIN_END,	"",
	"EXAM_BEGIN",	M_EXAM_BEGIN,	"#",
	"EXAM_END",	M_EXAM_END,	"",
	"JEXAM_BEGIN",	M_JEXAM_BEGIN,	"#",
	"JEXAM_END",	M_JEXAM_END,	"",
	"SET_SEC",	M_SET_SEC,	"#$",
	"APPENDIX",	M_APPENDIX,	"$",
	"APDX_BEGIN",	M_APDX_BEGIN,	"",
	"BLANK",	M_BLANK,	"",
	"PAGE",		M_PAGE,		"",
	"NEWLINE",	M_NEWLINE,	"",
	"CENTER_BEGIN",	M_CENTER_BEGIN,	"",
	"CENTER_END",	M_CENTER_END,	"",
	"RIGHT_BEGIN",	M_RIGHT_BEGIN,	"",
	"RIGHT_END",	M_RIGHT_END,	"",
	"INDENT",	M_INDENT,	"#",
	"INDENT0",	M_INDENT0,	"",
	"TITLE",	M_TITLE,	"&&&&&&&&&",
	"FOOTN_BEGIN",	M_FOOTN_BEGIN,	"",
	"FOOTN_END",	M_FOOTN_END,	"",
	"REFER_BEGIN",	M_REFER_BEGIN,	"",
	"REFER_END",	M_REFER_END,	"",
	"BOLD_BEGIN",	M_BOLD_BEGIN,	"",
	"BOLD_END",	M_BOLD_END,	"",
	"INDEX_BEGIN",	M_INDEX_BEGIN,	"",
	"INDEX_END",	M_INDEX_END,	"",
	"SECTION_END",	M_SECTION_END,	"",
	"SECTION_1",	M_SECTION_1,	"$",
	"SECTION_2",	M_SECTION_2,	"$",
	"SECTION_3",	M_SECTION_3,	"$",
	"SECTION_4",	M_SECTION_4,	"$",
	"SECTION_5",	M_SECTION_5,	"$",
	"SECTION",	M_SECTION,	"#$",
	"SETSEC_1",	M_SETSEC_1,	"#",
	"SETSEC_2",	M_SETSEC_2,	"#",
	"SETSEC_3",	M_SETSEC_3,	"#",
	"SETSEC_4",	M_SETSEC_4,	"#",
	"SETSEC_5",	M_SETSEC_5,	"#",
	"SETSEC",	M_SETSEC,	"##",
};

struct	cmpMac *outMacro[M_MAXNUM];

#define	ATYPE_VOID	0
#define	ATYPE_INT	1
#define	ATYPE_STR	2
#define	ATYPE_PSTR	3

static	int
atype(c)
char	c;
{
	if (c == '#')
		return ATYPE_INT;
	if (c == '$' || c == '@')
		return ATYPE_STR;
	if (c == '&')
		return ATYPE_PSTR;
	return ATYPE_VOID;
}

struct	cmpMac	*
macroParse(s, macroNum, orig)
int	macroNum;
char	*s;
char	*orig;
{
	struct	cmpMac	*mip, *new_mip;
	int	def_type;
	mip = (struct cmpMac *)malloc(sizeof(struct cmpMac));
	if(mip == NULL){ /* Add Nide */
	malloc_error:
		fprintf(stderr, "PANIC(malloc in macroParse)\n");
		exit (2);
	}
	mip->cmac_next = NULL;
	mip->cmac_argnum  = -1;
	mip->cmac_str = s;
	while (*s) {
		if (isdigit(*(s+1)) && (def_type = atype(*s)) != ATYPE_VOID) {
			new_mip=(struct cmpMac *)malloc(sizeof(struct cmpMac));
			if(new_mip == NULL) goto malloc_error; /* Add Nide */
			mip->cmac_next = new_mip;
			new_mip->cmac_next = NULL;
			new_mip->cmac_argtype = *s;
			new_mip->cmac_argnum  = *(s + 1) - '0';
			if (new_mip->cmac_argnum >
			    strlen(macNames[macroNum].mname_argattr)) {
				fprintf(stderr,"Argnum too big %d in \"%s %s\"\n",
					new_mip->cmac_argnum,
					macNames[macroNum].mname_name, orig);
				exit(1);
			}
			else if (atype(macNames[macroNum].
				       mname_argattr[new_mip->cmac_argnum - 1])
				 != def_type) {
				fprintf(stderr,"Improper argtype in \"%s %s\"\n",
					macNames[macroNum].mname_name, orig);
				exit(1);
			}
			*s = '\0';
			if (*(s + 2) == '\0')
				return mip;;
			new_mip->cmac_next = macroParse(s + 2, macroNum, orig);
			break;
		}
		s++;
	}
	return mip;
}
char	*
getMacroNum(buf, macroNump)
char	*buf;
int	*macroNump;
{
	char	*rval = NULL;
	char	*s = buf;
	int	i;
	*macroNump = -1;
	while (!isspace(*s))
		s++;
	if (*s != '\n')
		rval = s + 1;
	*s = '\0';
	for (i = 0; i < sizeof(macNames)/sizeof(struct macNames); i++) {
		if (strcmp(macNames[i].mname_name, buf) == 0) {
			*macroNump = i;
			return rval;
		}
	}
	return rval;
}
getMacroDef(buf, rstr)
char	*buf;
char	*rstr;
{
	int in_escape = 0;
	rstr[0] = '\0';
	while (1) {
		if (!in_escape && *buf == '"') {
			*rstr = '\0';
			return 0;
		}
		if (in_escape) {
			in_escape = 0;
			switch(*buf) {
			    case '\n':
				*rstr++ = '\0';
				return -2;
			    case 'n':
				*rstr++ = '\n';
				break;
			    case 't':
				*rstr++ = '\t';
				break;
			    default:
				*rstr++ = *buf;
				break;
			}
		}
		else if (*buf == '\n')
			return 1;
		else if (*buf == '\\')
			in_escape = 1;
		else {
			in_escape = 0;
			*rstr++ = *buf;
		}
		buf++;
	}
}
putMacro(macroNum, va_alist)
int	macroNum;
va_dcl
{
	va_list	ap;
	union	macroArg {
		int	ma_num;
		char	*ma_str;
		char	**ma_pstr;
	} margs[MACRO_MAXARG];
	int	argtype[MACRO_MAXARG];
	struct	cmpMac	*mip = outMacro[macroNum];
	int	i, maxarg = 0;

	for (i = 1; i < MACRO_MAXARG; i++)
		argtype[i] = ATYPE_VOID;
	if (mip == NULL)
		return -1;
	while(mip) {
		if (mip->cmac_argnum > 0) {
			if (maxarg < mip->cmac_argnum)
				maxarg = mip->cmac_argnum;
			switch(mip->cmac_argtype) {
			    case '#':
				argtype[mip->cmac_argnum] = ATYPE_INT;
				break;
			    case '$':
			    case '@':
				argtype[mip->cmac_argnum] = ATYPE_STR;
				break;
			    case '&':
				argtype[mip->cmac_argnum] = ATYPE_PSTR;
				break;
			}
		}
		mip = mip->cmac_next;
	}
	va_start(ap);
	for (i = 1; i <= maxarg; i++) {
		switch(argtype[i]) {
		    case ATYPE_VOID:
			va_arg(ap, char *);
			break;
		    case ATYPE_INT:
			margs[i].ma_num = (int)va_arg(ap, int *);
			break;
		    case ATYPE_STR:
			margs[i].ma_str = va_arg(ap, char *);
			break;
		    case ATYPE_PSTR:
			margs[i].ma_pstr = (char **)va_arg(ap, char **);
			break;
		}
	}
	mip = outMacro[macroNum];
	while(mip) {
		if (mip->cmac_argnum > 0) {
			switch(mip->cmac_argtype) {
			    case '#':
				printf ("%d", margs[mip->cmac_argnum].ma_num);
				break;
			    case '$':
				printf ("%s", codeCvt((*put->quote1)(margs[mip->cmac_argnum].ma_str)));
				break;
			    case '@':
				printf ("%s", codeCvt((*put->quote2)(margs[mip->cmac_argnum].ma_str)));
				break;
			    case '&':
				{
					char	**argp;
					argp = margs[mip->cmac_argnum].ma_pstr;
					while (*argp) {
						printf("%s", codeCvt((*put->quote1)(*argp)));
						if (*++argp) {
							if (put == &texPut) {
								printf("~\\\\");
							}
							putchar('\n');
						}
					}
					break;
				}
			}
		}
		else
			printf ("%s", codeCvt(mip->cmac_str));
		mip = mip->cmac_next;
	}
	va_end(ap);
	return 0;
}
clearMacro()
{
	int	i;
	for (i = 0; i < M_MAXNUM; i++)
		outMacro[i] = NULL;
}
initMacroDefs(mcp)
struct	macDefs	*mcp;
{
	while(mcp->mdef_number >= 0) {
		if (outMacro[mcp->mdef_number] == NULL)
			outMacro[mcp->mdef_number] =
				macroParse(mcp->mdef_def, mcp->mdef_number,
					   mcp->mdef_def);
		mcp++;
	}
}

FILE	*
macroFopen(fname)
char	*fname;
{
	FILE	*fd;
	char	buf[1024];
	if ((fd = fopen(fname, "r")) != NULL)
		return fd;
	strcpy(buf, fname);
	strcat(buf, MACRO_SUFFIX);
	return fopen(buf, "r");
}

macroFile(fname)
char	*fname;
{
	char	defstr[1024];
	char	buf[1024];
	char	path[1024];
	FILE	*fd;
	int	macroNum;
	char	*s;
	fd = macroFopen(fname);
	if (fd == NULL && plain2Lib != NULL && fname[0] != '/') {
		char	*colon;
		fd = NULL;

		colon = index(plain2Lib, ':');
		while (colon && colon != NULL) {
			*colon = '\0';
			sprintf(path, "%s/%s", plain2Lib, fname);
			if ((fd = macroFopen(path)) != NULL)
				break;
			plain2Lib = colon+1;
			colon = index(plain2Lib, ':');
		}
		if (fd == NULL && *plain2Lib != '\0') {
			sprintf(path, "%s/%s", plain2Lib, fname);
			fd = macroFopen(path);
		}
	}
	if (fd == NULL) {
#ifdef	MACRO_LIB
		sprintf(path, "%s/%s", MACRO_LIB, fname);
		if ((fd = macroFopen(path)) == NULL) {
			fprintf(stderr, "Can't open %s\n", fname);
			exit(1);
		}
#else
		fprintf(stderr, "Can't open %s\n", fname);
		exit(1);
#endif
	}
	while(fgets(buf, sizeof(buf), fd) != NULL) {
		char	*macro_body;
		int	ret;
		if (buf[0] == '#')
			continue;
		macro_body = getMacroNum(buf, &macroNum);
		if (macroNum < 0 || macro_body == NULL)
			continue;
		while (isspace(*macro_body))
			macro_body++;
		if (*macro_body != '"') {
			fprintf(stderr,"Macro body must start with \" in \"%s\"\n",
				macNames[macroNum].mname_name);
			fclose(fd);
			exit(1);
		}
		macro_body++;
		ret = getMacroDef(macro_body, defstr);
		while(ret == -2) {
			for (s = defstr; *s; s++);
			if (fgets(buf, sizeof(buf), fd) == NULL)
				break;
			ret = getMacroDef(buf, s);
		}
		if (ret == -1)
			continue;
		s = strsave(defstr); /* Changed Nide (but don't we need free?)*/
		outMacro[macroNum] = macroParse(s, macroNum, s);
	}
	fclose(fd);
	return 0;
}

/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: roffout.c,v 2.16 1994/04/19 10:17:03 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
#include "picture.h"
#include "table.h"
#include "macro.h"
					    
struct	macDefs roffMacros[] = {
	M_DOC_BEGIN,	".nr Ls 0\n.S #1 #1+2\n",
	M_DOC_END,	"",
	M_PLAIN_BEGIN,	".P\n",
	M_PLAIN_END,	"",
	M_EXAM_BEGIN,	".ft CW\n.vs -2\n.nf\n",
	M_EXAM_END,	".fi\n.vs +2\n.ft\n",
	M_APPENDIX,	"\\fBAppendix $1\\fP\n",
	M_BLANK,	"\n",
	M_PAGE,		".bp\n",
	M_NEWLINE,	".br\n",
	M_CENTER_BEGIN,	".ce 9999\n",
	M_CENTER_END,	".ce 0\n",
	M_RIGHT_BEGIN,	".ad r\n",
	M_RIGHT_END,	".ad b\n",
	M_INDENT,	".in #1/2u\n",
	M_INDENT0,	".in 0\n",
	M_FOOTN_BEGIN,	"\\*F\n.FS\n",
	M_FOOTN_END,	".FE\n",
	M_BOLD_BEGIN,	"\\fB",
	M_BOLD_END,	"\\fP",
	M_SECTION,	".H #1 \"@2\"\n",
	M_SETSEC,	".nr H#1 #2\n",
	M_TITLE,	".ce 999\n\\fB\\s+4&1\n\\fP\\s-4\n.ce 0\n\
.ad r\n&3\n.br\n&4\n.br\n&5\n.br\n&6\n.br\n&7\n.br\n.ad b\n.br 2\n",
	-1,	"",
};

struct	macDefs roffMsMacros[] = {
	M_SECTION,	".NH #1\n @2\n",
	M_SECTION_1,	".NH\n @1\n",
	M_DOC_BEGIN,	".nr PI 2n\n.ps #1\n.vs #1+2\n",
	M_PLAIN_BEGIN,	".PP\n",
	-1,	"",
};

/*
 * Roff text output routines
 */ 
#define	ROFF_TRANS	\
	{'\\',"\\e"},\
	{'\'',"\\'"},\
	{0,0}
struct transTable	roffTrans[] = {
	ROFF_TRANS
};
/*
 * Quote for double quoted strings
 *	 used for section and description list
 */
struct transTable	roffTransQ[] = {
	{'\"',"\\\""},
	ROFF_TRANS
};
char	*
roffQuote1(str)
char	*str;
{
	return codeCvt(textQuote(str, roffTrans));
}
char	*
roffQuote2(str)
char	*str;
{
	return codeCvt(textQuote(str, roffTransQ));
}
roffPlain(str, attr, newline)
char	*str;
int	attr;
int	newline;
{
	if (*str == '.')
		fputs("\\&", stdout);
	if (attr == IL_RAW)
		fputs(codeCvt(str), stdout);
	else
		fputs(roffQuote1(str), stdout);
	if (newline)
		fputs("\n", stdout);
}
roffExample(str)
char	*str;
{
	if (*str == '.')
		fputs("\\&", stdout);
	fputs(roffQuote1(str), stdout);
	putchar('\n');
}
#ifdef	KANJI
static	char	*bmarks[] = {"\\(bu", "¡¦", "¡þ" };
static	char	*dmarks[] = {"-", "¡Ý" };
#else
static	char	*bmarks[] = {"\\(bu"};
static	char	*dmarks[] = {"-"};
#endif
static	int	blevel = 0;
static	int	dlevel = 0;

#define	MAX_LIST_DEPTH	10
static	int	list_number[MAX_LIST_DEPTH];
static	int	list_type[MAX_LIST_DEPTH];
static	char	*list_format1[MAX_LIST_DEPTH];
static	char	*list_format2[MAX_LIST_DEPTH];

roffListBlock(begin, level, ltype, hint)
int	begin;
int	level;
int	ltype;
char	hint;
{
	switch (roffMacro) {
	    case MM_MACRO:
		mmListBlock(begin, level, ltype, hint);
		break;
	    case MS_MACRO:
		msListBlock(begin, level, ltype, hint);
		break;
	}
	
}
mmListBlock(begin, level, ltype, hint)
int	begin;
int	level;
int	ltype;
char	hint;
{
	int	i;
	if (begin) {
		switch (ltype) {
		    case L_BULLET:
			if (blevel < sizeof(bmarks)/sizeof(char *))
				i = blevel;
			else
				i = sizeof(bmarks)/sizeof(char *) - 1;
			printf(".ML %s\n", bmarks[i]);
			blevel++;
			break;
		    case L_DASH:
			if (dlevel < sizeof(dmarks)/sizeof(char *))
				i = dlevel;
			else
				i = sizeof(dmarks)/sizeof(char *) - 1;
			printf(".ML %s\n", dmarks[i]);
			dlevel++;
			break;
		    case L_DLIST:
			fputs(".VL 4 0\n", stdout);
			break;
		    case L_SROMAN:
			fputs(".AL i", stdout);
			goto moreStyle;
		    case L_LROMAN:
			fputs(".AL I", stdout);
			goto moreStyle;
		    case L_NUMBER:
			fputs(".AL 1", stdout);
			goto moreStyle;
		    case L_LALPHA:
			fputs(".AL A", stdout);
			goto moreStyle;
		    case L_SALPHA:
			fputs(".AL a", stdout);
			goto moreStyle;
		    moreStyle:
			if (listDecor) {
				switch (hint) {
				    case LH_PAREN:
					fputs(" ()\n", stdout);
					break;
				    case LH_RPAREN:
					fputs(" )\n", stdout);
					break;
				    case LH_BRACKET:
					fputs(" []\n", stdout);
					break;
				    case LH_RBRACKET:
					fputs(" ]\n", stdout);
					break;
				    case LH_DOTTED:
					fputs(" .\n", stdout);
					break;
				    default:
					fputs(" \n", stdout);
					break;
				}
			}
			else
				fputs("\n", stdout);
			break;
		    default:
			break;
		}
	}
	else {
		switch (ltype) {
		    case L_BULLET:
			blevel--;
			break;
		    case L_DASH:
			dlevel--;
			break;
		    default:
			break;
		}
		fputs(".LE\n", stdout);
	}
}
msListBlock(begin, level, ltype, hint)
int	begin;
int	level;
int	ltype;
char	hint;
{
	int	i;
	if (begin) {
		fputs(".in +2\n.sp .5\n", stdout);
		list_type[level]   = ltype;
		list_number[level] = 0;
		switch (ltype) {
		    case L_BULLET:
			if (blevel < sizeof(bmarks)/sizeof(char *))
				i = blevel;
			else
				i = sizeof(bmarks)/sizeof(char *) - 1;
			list_format1[level] = bmarks[i];
			blevel++;
			break;
		    case L_DASH:
			if (dlevel < sizeof(dmarks)/sizeof(char *))
				i = dlevel;
			else
				i = sizeof(dmarks)/sizeof(char *) - 1;
			list_format1[level] = dmarks[i];
			dlevel++;
			break;
		    case L_DLIST:
			break;
		    default:
			switch (hint) {
			    case LH_PAREN:
				list_format1[level] = "(";
				list_format2[level] = ")";
				break;
			    case LH_RPAREN:
				list_format1[level] = "";
				list_format2[level] = ")";
				break;
			    case LH_BRACKET:
				list_format1[level] = "[";
				list_format2[level] = "]";
				break;
			    case LH_RBRACKET:
				list_format1[level] = "";
				list_format2[level] = "]";
				break;
			    case LH_DOTTED:
				list_format1[level] = "";
				list_format2[level] = ".";
				break;
			    default:
				list_format1[level] = "";
				list_format2[level] = "";
				break;
			}
			break;
		}
	}
	else {
		switch (ltype) {
		    case L_BULLET:
			blevel--;
			break;
		    case L_DASH:
			dlevel--;
			break;
		    default:
			break;
		}
		fputs(".in -2\n", stdout);
	}
}
roffDlistItem(level, dscr, cont)
int	level;
char	*dscr;
int	cont;
{
	switch (roffMacro) {
	    case MM_MACRO:
		printf(".LI \"\\fB%s\\fP\" \n", roffQuote2(dscr));
		break;
	    case MS_MACRO:
		fputs(".ti -1\n", stdout);
		printf("\\fB%s\\fP \n", roffQuote2(dscr));
		break;
	}
}
static char *lroman_list[] = {
	"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X"
	};
static char *sroman_list[] = {
	"i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix", "x"
	};
roffListItem(level)
int	level;
{
	switch (roffMacro) {
	    case MM_MACRO:
		fputs(".LI\n", stdout);
		break;
	    case MS_MACRO:
		fputs(".ti -2\n", stdout);
		switch (list_type[level]) {
		    case L_BULLET:
		    case L_DASH:
			printf("%s ", list_format1[level]);
			break;
		    case L_NUMBER:
			printf("%s%d%s ", list_format1[level], ++list_number[level],
			       list_format2[level]);
			break;
		    case L_LROMAN:
			printf("%s%s%s ",
			       list_format1[level],
			       lroman_list[list_number[level]++],
			       list_format2[level]);
			if (list_number[level] >= sizeof(lroman_list) / sizeof(char *))
				list_number[level]--;
			break;
		    case L_SROMAN:
			printf("%s%s%s ", 
			       list_format1[level],
			       sroman_list[list_number[level]++],
			       list_format2[level]);
			if (list_number[level] >= sizeof(sroman_list) / sizeof(char *))
				list_number[level]--;
			break;
		    case L_SALPHA:
			printf("%s%c%s ", list_format1[level],
			       'a' + list_number[level]++,
			       list_format2[level]);
			break;
		    case L_LALPHA:
			printf("%s%c%s ", list_format1[level],
			       'A' + list_number[level]++,
			       list_format2[level]);
		    default:
			break;
		}
		break;
	}
}
roffSpace(length)
int	length;
{
	printf(".DS I\n");
	printf(".vs 13\n");
	printf(".sp %d\n", length);
	printf(".vs\n");
	printf(".DE\n");
}
roffRawText(str)
char	*str; {
	printf("%s\n", codeCvt(str));
}
roffFTitle(ft, str, capsule)
int	ft;
char	*str;
int	capsule;
{
	static int	figNumber = 1;
	static int	tblNumber = 1;
	fputs ("\\s-2", stdout);
#ifdef	KANJI
	if (japaneseText) {
		if (ft == FT_FIGURE)
			printf ("\\fB¿Þ %d.\\fP", figNumber++);
		else if (ft == FT_TABLE)
			printf ("\\fBÉ½ %d.\\fP", tblNumber++);
	}
	else {
#endif
		if (ft == FT_FIGURE)
			printf ("\\fBFigure %d.\\fP", figNumber++);
		else if (ft == FT_TABLE)
			printf ("\\fBTable %d.\\fP", tblNumber++);
#ifdef	KANJI
	}
#endif
	printf (" %s\\s0\n", roffQuote1(str));
}
#ifdef	PICTURE
/*
 * Driver for roff (pic) output
 */
static int	picLines;		/* Number of lines in the block	*/
static int	picMinInd;		/* Indentation of the region	*/
int	roffVsize;
extern struct transTable	roffTransQ[];
#define	ROFF_SCALE	134.
#define	ROFF_Y(y)	((roffVsize - (y)) * picFontSize / ROFF_SCALE)
#define	ROFF_X(x)	((x - picMinInd) * picFontSize / ROFF_SCALE)
#define	ROFF_LEN(l)	((l) * picFontSize / ROFF_SCALE)
roffPictureBlock(begin, lines, indent, maxLen)
int	begin;
int	lines;
int	indent;
int	maxLen;
{
	picLines  = lines;
	picMinInd = indent;
	if (begin) {
		roffVsize = lines * 2;
		fputs(".PS\n", stdout);
	}
	else {
		printf(".ps %d\n", picFontSize);
		fputs(".PE\n", stdout);
	}
}
/* set line width	*/
roffLineWidth(style)
int	style;
{
	if (style == SY_DBL)
		printf(".ps %d\n", fontSize * 2);
	else if (style == SY_THICK)
		printf(".ps %d\n", fontSize<14 ? 14 : fontSize);
	else
		printf(".ps %d\n", fontSize / 2);
}
roffPicLine(x0, y0, x1, y1, style, vector)
int	x0, x1, y0, y1;
int	style;
int	vector;
{
	DBG4(4, "roffPicLine (%d,%d)-(%d,%d)\n", x0, y0, x1, y1);
	roffLineWidth(style);
	if (style == SY_ELPS || style == SY_HELPS) {
		printf("ellipse wid %f ht %f at %f, %f\n",
			ROFF_LEN(x1 - x0), ROFF_LEN((x1 - x0)/4.),
			ROFF_X(x0 + (x1 - x0) / 2.),
			ROFF_Y(y0 + (y1 - y0) / 2.));
		return;
	}
	if (style == SY_CIRCLE) {
		printf("circle radius %f at %d, %d\n",
		       ROFF_LEN(abs(x1 - x0) / 2.),
		       ROFF_X((x1 + x0) / 2), ROFF_Y(y0 + (x1 - x0) / 2));
		return;
	}
	fputs("line", stdout);
	switch (vector) {
	    case VECT_BGN:
		fputs(" <-", stdout);
		break;
	    case VECT_END:
		fputs(" ->", stdout);
		break;
	    case VECT_BOTH:
		fputs(" <->", stdout);
		break;
	    default:
		break;
	}
	printf(" from %f, %f to %f, %f",
	       ROFF_X(x0), ROFF_Y(y0), ROFF_X(x1), ROFF_Y(y1));
	if (style == SY_DASH)
		printf(" dashed 0.%02di\n",picFontSize / 4);
	else
		fputs("\n", stdout);
}
roffPicArc(x, y, r, dir, style)
int	x, y, r;
enum	direction dir;
int	style;
{
	int	x0, x1, y0, y1;
	float	offset;
	x0 = x1 = x;
	y0 = y1 = y;
	switch (dir) {
	    case dir_br:
		x0 = x + r;
		y1 = y + r;
		break;
	    case dir_bl:
		x1 = x - r;
		y0 = y + r;
		break;
	    case dir_tl:
		x0 = x - r;
		y1 = y - r;
		break;
	    case dir_tr:
		x1 = x + r;
		y0 = y - r;
		break;
	}
	roffLineWidth(style);
	if (style == SY_THICK)
		offset = (fontSize / 20) / ROFF_SCALE;
	else
		offset = 0;
	printf("arc cw from %f, %f to %f, %f rad %f\n",
	       ROFF_X(x0) + offset, ROFF_Y(y0) - offset,
	       ROFF_X(x1) + offset, ROFF_Y(y1) - offset, ROFF_LEN(r));
/*	       ROFF_X(x0), ROFF_Y(y0), ROFF_X(x1), ROFF_Y(y1),ROFF_LEN(r));*/
}
roffPicText(s, xpos, vpos, factor)
char	*s;
int	xpos;
int	vpos;
int	factor;
{
	if (factor < 10)
		factor = 10;
	printf(".ps %d\n",
	       picFontSize * 10 / factor);
	printf("\"%s\" ljust at %f, %f\n", roffQuote2(s),
	       ROFF_X(xpos), ROFF_Y(vpos + 1));
}
#endif
roffTitle(style, haifu, title, shozoku, number, date, name, renraku
#ifdef	TITLE_SPECIAL
			 ,special8, special9
#endif
	  )
int	style;
char	**haifu;
char	**title;
char	**shozoku;
char	**number;
char	**date;
char	**name;
char	**renraku;
#ifdef	TITLE_SPECIAL
char	**special8, **special9;
#endif
{
	char	**s;
	if (style == 0) {
		putMacro(M_TITLE,
			 title, number, date, shozoku, haifu, name, renraku
#ifdef	TITLE_SPECIAL
			 ,special8, special9
#endif
			 );
		
		return;
	}
#ifdef	NEC_CCS
	fputs(".4H\n.ce 0\n", stdout);
	for (s = haifu; *s != NULL; s++)
		printf("%s\n", roffQuote1(*s));
	fputs(".4E\n", stdout);
	fputs(".4T\n", stdout);
	for (s = title; *s != NULL; s++)
		printf("%s\n", roffQuote1(*s));
	fputs(".4E\n", stdout);
	fputs(".4O\n", stdout);
	for (s = shozoku; *s != NULL; s++)
		printf("%s\n", roffQuote1(*s));
	fputs(".4E\n", stdout);
	fputs(".4S\n", stdout);
	for (s = number; *s != NULL; s++)
		printf("%s\n", roffQuote1(*s));
	fputs(".4E\n", stdout);
	fputs(".4D\n", stdout);
	for (s = date; *s != NULL; s++)
		printf("%s\n", roffQuote1(*s));
	fputs(".4E\n", stdout);
	fputs(".4N\n", stdout);
	for (s = name; *s != NULL; s++)
		printf("%s\n", roffQuote1(*s));
	fputs(".4E\n", stdout);

	if (style == TITLE_A47)
		fputs(".47\n", stdout);
	if (style == TITLE_A47_1)
		fputs(".471\n", stdout);

	if (*renraku != NULL) {
		fputs(".ad r\n", stdout);
		for (s = renraku; *s != NULL; s++)
			printf("%s\n", roffQuote1(*s));
		fputs(".br\n.ad b\n", stdout);
	}
#endif
}
roffTable(begin, end, tblp)
int	begin;
int	end;
struct	table	*tblp;
{
	int	j, l, fld;
	char	*atr;
	int	hattr;
	int	exCol;
	short	curAttr, prevAttr = HORI_NULL;
	fputs(".TS", stdout);
	for (l = begin; l < end; l++) {
		if ((hattr = lineAttr(texts[l]->body, &atr))
		    == HORI_DBL_ALL || hattr == HORI_SNGL_ALL)
			continue;
		fputs("\n", stdout);
		for (j = 0; j < tbl_field[0].vlines; j++)
			fputs("|", stdout);
		for (fld = 1; fld < tbl_nfield; fld += exCol + 1) {
			exCol = extraColumn(atr, texts[l]->length, fld, tblp);
			putchar(formatOf(atr, texts[l]->length, fld, exCol, tblp));
			for (j = 0; j < exCol; j++)
				fputs("s", stdout);

			curAttr = attrOfStr(atr, texts[l]->length, fld, exCol, tblp);
			if (prevAttr != HORI_SNGL_ALL
			    || curAttr != HORI_SNGL_ALL)
				for (j = 0; j < tbl_field[fld + exCol].vlines; j++)
					fputs("|", stdout);
			prevAttr = curAttr;
		}
	}
	fputs(".\n", stdout);
	for (l = begin; l < end; l++) {
		if ((hattr = lineAttr(texts[l]->body, &atr)) == HORI_SNGL_ALL){
			fputs("_\n", stdout);
			continue;
		}
		else if (hattr == HORI_DBL_ALL) {
			fputs("=\n", stdout);
			continue;
		}
		for (fld = 1; fld < tbl_nfield; fld += exCol + 1) {
			exCol = extraColumn(atr, texts[l]->length, fld, tblp);
			if (attrOfStr(atr, texts[l]->length, fld, exCol, tblp)
			    == HORI_SNGL_ALL)
				fputs("_\t", stdout);
			else
				printf("%s\t", roffQuote1(tblSubstr(l, fld, exCol, tblp)));
		}
		fputs("\n", stdout);
	}
	fputs(".TE\n", stdout);
	fputs(".in\n", stdout);
}
roffCapsule(begin, type, center)
int	begin;
int	type;
{
	if (begin) {
		if (center)
			fputs(".DS CB\n", stdout);
		else
			fputs(".DS\n", stdout);
	}
	else {
		fputs(".DE\n", stdout);
	}
}
roffComment(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end; l++) {
		printf ("' %s\n", codeCvt(texts[l]->body));
	}
}

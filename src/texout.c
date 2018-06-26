/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: texout.c,v 2.20 1994/04/19 10:17:09 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
#include "picture.h"
#include "table.h"
#include "macro.h"

#define	STR_DOC_BEGIN	"\
\\newif\\ifNTT\\NTTfalse\n\
\\ifx\\gtfam\\undefined\\ifx\\gtfamily\\undefined\\NTTtrue\\fi\\fi\n\
\\ifNTT\\documentstyle[$2]{j-article}\n\
\\else\\documentstyle[$3]{jarticle}\\fi\n\
\\setcounter{secnumdepth}{6}\n\
\\setcounter{tocdepth}{6}\n\
\\topsep=0.1cm\n\
\\parsep=0.1cm\n\
\\itemsep=0.0cm\n\
%\\renewcommand{\\bf}{\\protect\\pbf\\protect\\pdg}\n\
\\begin{document}\n"

struct	macDefs texMacros[] = {
	M_DOC_BEGIN,	STR_DOC_BEGIN,
	M_DOC_END,	"\\end{document}\n",
	M_PLAIN_BEGIN,	"\\par\n",
	M_PLAIN_END,	"",
	M_EXAM_BEGIN,	"{\\par\\baselineskip=#1pt\n\\begin{verbatim}\n",
	 /* 直前の行間まで狭まってしまうのを防ぐため\parを置いた */
	M_EXAM_END,	"\\end{verbatim}}\n",
	M_JEXAM_BEGIN,	"{\\par\\baselineskip=#1pt\n\\begin{jverbatim}\n",
	M_JEXAM_END,	"\\end{jverbatim}\\par}\n",
	 /* jverbatimの前後には\par必要。jverb.styのドキュメント参照 */
	M_APDX_BEGIN,	"\\appendix\n",
	M_APPENDIX,	"\\section{@1}\n",
	M_BLANK,	"\\medskip\n",
	M_PAGE,		"\\newpage\n",
	M_NEWLINE,	"\\\\\n",
	M_CENTER_BEGIN,	"\\begin{center}\n",
	M_CENTER_END,	"\\end{center}\n",
	M_RIGHT_BEGIN,	"\\begin{flushright}\n",
	M_RIGHT_END,	"\\end{flushright}\n",
	M_INDENT,	"{\\list{}{\\leftmargin=#1ex}\\item[]\n",
	M_INDENT0,	"\\endlist}\n",
	M_FOOTN_BEGIN,	"\\footnote{",
	M_FOOTN_END,	"}\n",
	M_REFER_BEGIN,	"\\ref{",
	M_REFER_END,	"}",
	M_BOLD_BEGIN,	"{\\bf ",
	M_BOLD_END,	"}\n",
	M_INDEX_BEGIN,	"\\index{",
	M_INDEX_END,	"}\n",
	M_SECTION_1,	"\n\\section{@1}\n",
	M_SECTION_2,	"\n\\subsection{@1}\n",
	M_SECTION_3,	"\n\\subsubsection{@1}\n",
	M_SECTION_4,	"\n\\paragraph{@1}~\\\\\n",
	M_SECTION_5,	"\n\\subparagraph{@1}~\\\\\n",
	M_SECTION,	"\n\\subparagraph{@2}~\\\\\n",
	M_SETSEC_1,	"\n\\setcounter{section}{#1}\n",
	M_SETSEC_2,	"\n\\setcounter{subsection}{#1}\n",
	M_SETSEC_3,	"\n\\setcounter{subsubsection}{#1}\n",
	M_SETSEC_4,	"\n\\setcounter{paragraph}{#1}\n",
	M_SETSEC_5,	"\n\\setcounter{subparagraph}{#1}\n",
	M_TITLE,	"\\title{\n&1}\n\\author{\n&6\\\\\n&4}\n\\date{\n&3}\n\\maketitle\n",
	-1,	"",
	};

/*
 * TeX text output routines
 */ 

struct transTable	*texTrans;
struct transTable	*texTransM;

static struct transTable	texFullTrans[] = {
/*	{'-', "$-$" },*/
	{'=', "$=$" },
	{'|', "$|$" },
	{'<', "$<$" },
	{'>', "$>$" },
	{'#', "\\#" },
	{'$', "\\$" },
	{'%', "\\%" },
	{'&', "\\&" },
	{'~', "\\verb+~+" },
	{'_', "\\_" },
	{'^', "\\verb+^+" },
	{'{', "\\{" },
	{'}', "\\}" },
	{'\\', "$\\backslash$" },
	{0,0} };
static struct transTable	texFullTransM[] = {
/*	{'-', "$-$" },*/
	{'=', "$=$" },
	{'|', "$|$" },
	{'<', "$<$" },
	{'>', "$>$" },
	{'#', "\\#" },
	{'$', "\\$" },
	{'%', "\\%" },
	{'&', "\\&" },
	{'*', "$*$" },
	{'~', "{\\tt \\symbol{126}}" },
	{'_', "\\_" },
	{'^', "{\\tt \\symbol{94}}" },
	{'{', "\\{" },
	{'}', "\\}" },
	{'[', "$[$" },
	{']', "$]$" },
	{'\\', "$\\backslash$" },
	{'*', "\\*" },
	{0,0} };

static struct transTable	texHalfTrans[] = {
	{'=', "$=$" },
	{'|', "$|$" },
	{'<', "$<$" },
	{'>', "$>$" },
	{'#', "\\#" },
	{'$', "\\$" },
	{'%', "\\%" },
	{'&', "\\&" },
	{'~', "\\verb+~+" },
	{'_', "\\_" },
	{'^', "\\verb+^+" },
	{0,0} };

static struct transTable	texHalfTransM[] = {
/*	{'-', "$-$" },*/
	{'=', "$=$" },
	{'|', "$|$" },
	{'<', "$<$" },
	{'>', "$>$" },
	{'#', "\\#" },
	{'$', "\\$" },
	{'%', "\\%" },
	{'&', "\\&" },
	{'*', "$*$" },
	{'~', "{\\tt \\symbol{126}}" },
	{'_', "\\_" },
	{'^', "{\\tt \\symbol{94}}" },
	{'*', "\\*" },
	{0,0} };

texSetTrans(full)
int	full;
{
	if (full) {
		texTrans  = texFullTrans;
		texTransM = texFullTransM;
	}
	else {
		texTrans  = texHalfTrans;
		texTransM = texHalfTransM;
	}
}


char	*
texTextQuote(str, quotable)
char	*str;
int	quotable;
{
	char	*s;
	int	len;
	static	char	buf[MAX_LINE_LEN];
	if (rawOutput)
		return str;
	if (!quotable) 
		return textQuote(str, texTransM);
	else if (!texQuote)
		return textQuote(str, texTrans);
	s = buf;
	while(*str) {
		if (len = alpha(str)) {
			char	*p = useJverb ? "\\jverb|" : "\\verb|";
			(void)strcpy(s, p);
			s += strlen(p);
			(void)strncpy(s, str, len);
			s += len;
			str += len;
			(void)strcpy(s, "|");
			s += strlen("|");
		}
		else {
			textQuoteChar(&s, str, texTrans);
			str++;
		}
	}
	*s = '\0';
	return buf;
}
char	*texQuote1(str)
char	*str;
{
	return texTextQuote(str, 1);
}
char	*texQuote2(str)
char	*str;
{
	return texTextQuote(str, 0);
}
#define	TEX_MAXSECDEPTH	3
#define	TEX_MAXPARDEPTH	2
alpha(s)
char	*s;
{
	int	len, stat;
	len  = 0;
	stat = 0;
	while(*s){
		if (*s < '!' || *s > '}' && *s != '|')
			break;
		if ((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z'))
			stat = 1;
		len++;
		s++;
	}
	if (stat)
		return len;
	else
		return 0;
}
texPutLabel(str)
char	*str;
{
	printf("\\label{%s}\n", codeCvt(str));
}

texPlain(str, attr, newline)
char	*str;
int	attr;
int	newline;
{
	if (attr == IL_RAW || attr == IL_REFERENCE)
		printf("%s", codeCvt(str));
	else
		printf("%s", codeCvt(texTextQuote(str, 1)));
	if (newline)
		printf("\n");
}
texExample(str)
char	*str;
{
	printf("%s\n",codeCvt(str));
}
static char	*levelStr[] = {
	"i", "ii", "iii", "iv"
};
static int	enumLevel = 0;
texListBlock(begin, level, ltype, hint)
int	begin;
int	level;
int	ltype;
char	hint;
{
	char	*lstr;
	if (begin) {
		switch (ltype) {
		    case L_BULLET:
		    case L_DASH:
			printf("\\begin{itemize}\n");
			break;
		    case L_DLIST:
			printf("\\begin{description}\n");
			break;
		    case L_LROMAN:
		    case L_SROMAN:
		    case L_NUMBER:
		    case L_LALPHA:
		    case L_SALPHA:
			if (enumLevel < sizeof(levelStr)/sizeof(char *))
				lstr = levelStr[enumLevel];
			else {
				fprintf(stderr, "\nWarning: too many list nesting\n");
				lstr = "iv";
			}
			enumLevel++;
			if (listDecor) {
				char *left_paren = "" , *right_paren = "";
				printf("{\n");
				printf("\\renewcommand{\\theenum%s}", lstr);
				switch (hint) {
				    case LH_PAREN:
					left_paren = "(";
					right_paren = ")";
					break;
				    case LH_RPAREN:
					right_paren = ")";
					break;
				    case LH_BRACKET:
					left_paren = "[";
					right_paren = "]";
					break;
				    case LH_RBRACKET:
					right_paren = "]";
					break;
				    case LH_DOTTED:
					right_paren = ".";
					break;
				}
				printf ("{%s", left_paren);
				switch (ltype) {
				    case L_LROMAN:
					printf("\\Roman{enum%s}",lstr);
					break;
				    case L_SROMAN:
					printf("\\roman{enum%s}",lstr);
					break;
				    case L_NUMBER:
					printf("\\arabic{enum%s}",lstr);
					break;
				    case L_LALPHA:
					printf("\\Alph{enum%s}",lstr);
					break;
				    case L_SALPHA:
					printf("\\alph{enum%s}",lstr);
					break;
				}
				printf ("%s}\n", right_paren);
				printf("\\renewcommand{\\labelenum%s}{\\theenum%s}\n",
				       lstr, lstr);
			}
			printf("\\begin{enumerate}\n");
			break;
		    default:
			break;
		}
	}
	else {
		switch (ltype) {
		    case L_BULLET:
		    case L_DASH:
			printf("\\end{itemize}\n");
			break;
		    case L_LROMAN:
		    case L_SROMAN:
		    case L_NUMBER:
		    case L_LALPHA:
		    case L_SALPHA:
			enumLevel--;
			printf("\\end{enumerate}\n");
			if (listDecor)
				printf("}\n");
			break;
		    case L_DLIST:
			printf("\\end{description}\n");
			break;
		    default:
			break;
		}
	}
}
texDlistItem(level, dscr, cont)
int	level;
char	*dscr;
int	cont;
{
	if (*dscr == '[' && *(dscr + strlen(dscr) -1 ) == ']') {
		dscr[strlen(dscr) - 1] = '\0';
		dscr++;
		printf("\\item[{[}%s{]}]", codeCvt(texTextQuote(dscr, 1)));
	}
	else
		printf("\\item[%s]", codeCvt(texTextQuote(dscr, 1)));
	if (!cont)
		printf("~\n");
}
texListItem(level)
int	level;
{
	printf("\\item ");
}
texRawText(str)
char	*str;
{
	printf("%s\n",codeCvt(str));
}
texSpace(length)
int	length;	
{
	printf("~\\\\\n\\begin{picture}(100,%d)\n", length * 15);
	printf("\\end{picture}\\\\\n");
}
texFTitle(ft, str, capsule)
int	ft;
char	*str;		/* if (str!=NULL) output title	*/
int	capsule;
{
	static int	figNumber = 1;
	static int	tblNumber = 1;
	if (crossRefer && capsule) {
		printf("\\center\\caption{%s}", codeCvt(texTextQuote(str, 1)));
		texPutLabel(str);
	}
	else {
		printf ("~\\\\\n");
#ifdef	KANJI
		if (japaneseText) {
			if (ft == FT_FIGURE)
				printf ("{\\bf %s %d.}", codeCvt("図"), figNumber++);

			else if (ft == FT_TABLE)
 				printf ("{\\bf %s %d.}", codeCvt("表"), tblNumber++);
		}
		else {
#endif
			if (ft == FT_FIGURE)
				printf ("{\\bf Figure %d.}", figNumber++);
			else if (ft == FT_TABLE)
				printf ("{\\bf Table %d.}", tblNumber++);
#ifdef	KANJI
		}
#endif
		printf (" %s\\\\\n", codeCvt(texTextQuote(str, 1)));
	}
}
#ifdef	PICTURE
/*
 * Driver for TeX(picture) output
 */
static int	picLines;		/* Number of lines in the block	*/
static int	picMinInd;		/* Indentation of the region	*/
#define	TEX_LEN(l)	((l) * texUnit)
#define	TEX_Y(y)	((picLines * 2 - (y)) * texUnit + 10)
#define	TEX_X(x)	((x) * texUnit)
#define	TEX_LINE(x0, y0, xdir, ydir, len)\
{\
	 printf("\\put(%f, %f)", (float)TEX_X(x0), (float)TEX_Y(y0));\
	 printf("{\\line(%d, %d){%f}}\n", xdir, ydir, TEX_LEN(len));\
 }\

static int	texVsize;
static int	texUnit;
texPictureBlock(begin, lines, indent, maxLen)
int	begin;
int	lines;
int	indent;
int	maxLen;
{
	picLines  = lines;
	picMinInd = indent;
	if (begin) {
		texUnit = picFontSize * 2 / 3;
		texVsize = (lines * 2 + 4) * texUnit;
		printf("\\begin{picture}(%d,%d)(0,0)\n",
		       maxLen * texUnit,
		       texVsize);
	}
	else {
		printf("\\end{picture}\n");
	}
}
/* set line width	*/
texLineWidth(style)
int	style;
{
	if (style == SY_DBL)
		printf("\\linethickness{%dpt}\n", texUnit / 2);
	else if (style == SY_THICK)
		printf("\\thicklines\n");
	else
		printf("\\thinlines\n");
}
texEllipse(x0, y0, x1, y1, style)
int	x0, x1, y0, y1;
int	style;
{
	static	float	texElpY[]    = { 0, 1/8., 1/6., 1/8.};
	static	short	texElpDirX[] = { 2,  6, 6, 2};
	static	short	texElpDirY[] = {-1, -1, 1, 1};
	int	len, ydir, i;
	DBG4(4, "texEllipse (%d,%d)-(%d,%d)\n", x0, y0, x1, y1);
	len = x1 - x0;
	for (ydir = 1; ydir >= ((style == SY_HELPS)?0:-1); ydir -= 2) {
		if (TEX_LEN(len) / 2 < 10) {
			texPicLine(x0, y0, x1, y1, SY_THIN, 0);
		}
		else if (TEX_LEN(len) / 4 < 10) {
			TEX_LINE(x0, y0, 4, -ydir, len / 2.);
			TEX_LINE(x0 + len / 2., y0 + len * ydir / 8.,
				 4, ydir, len / 2.);
		}
		else for (i = 0; i < 4; i++) {
			TEX_LINE(x0 + len * i / 4.,
				 y0 + (len * texElpY[i]) * ydir,
				 texElpDirX[i], texElpDirY[i] * ydir,
				 len / 4.);
		}
	}
}
/*
 * Draw line
 *	Only ({-1,0,1},{-1,0,1}) direction.
 */
texPicLine(x0, y0, x1, y1, style, vector)
int	x0, x1, y0, y1;
int	style;
int	vector;
{
	int	xdir, ydir, len;
	DBG4(4, "texPicLine (%d,%d)-(%d,%d)\n", x0, y0, x1, y1);
	texLineWidth(style);
	if (style == SY_ELPS || style == SY_HELPS) {
		texEllipse(x0, y0, x1, y1, style);
		return 0;
	}
	if (style == SY_CIRCLE) {
		printf("\\put(%d, %d){\\circle{%d}}\n",
		       TEX_X((x0 + x1) / 2), TEX_Y(y0 - (x1 - x0) / 2),
		       TEX_LEN((x1 - x0) / 2));
		return 0;
	}
	if (x0 == x1) {
		xdir = 0;
		ydir = sign(y0 - y1);
		len = abs(y1 - y0);
	}
	else if (y0 == y1) {
		xdir = sign(x1 - x0);
		ydir = 0;
		len = abs(x1 - x0);
	}
	else {
		xdir = x1 - x0;
		ydir = y0 - y1;
		if ((xdir % ydir) == 0) {
			xdir /= abs(ydir);
			ydir /= abs(ydir);
		}
		else if ((ydir % xdir) == 0) {
			ydir /= abs(xdir);
			xdir /= abs(xdir);
		}
		len = abs(x1 - x0);
	}
	DBG3(4, "  dir (%d,%d), len(%d)\n", xdir, ydir, len);
	if (style == SY_DASH) {
		switch (vector) {
		    case VECT_BOTH:
			printf("\\put(%d, %d){\\vector(%d,%d){%d}}\n",
			       TEX_X(x1), TEX_Y(y1), xdir, ydir, 0);
		    case VECT_BGN:
			printf("\\put(%d, %d){\\vector(%d,%d){%d}}\n",
			       TEX_X(x0), TEX_Y(y0), -xdir, -ydir, 0);
			goto	dashLine;
		    case VECT_END:
			printf("\\put(%d, %d){\\vector(%d,%d){%d}}\n",
			       TEX_X(x1), TEX_Y(y1), xdir, ydir, 0);
		    default:;
		    dashLine:
			printf("\\put(%d, %d){\\dashbox{1}(%d,%d){}}\n",
			       TEX_X(x0), TEX_Y(y1),
			       TEX_LEN(x1 - x0), TEX_LEN(y1 - y0));
			break;
		}
	}
	else {
		switch (vector) {
		    case VECT_BOTH:
			printf("\\put(%d, %d){\\vector(%d,%d){%d}}\n",
			       TEX_X(x1), TEX_Y(y1), xdir, ydir, 0);
		    case VECT_BGN:
			printf("\\put(%d, %d){\\vector(%d,%d){%d}}\n",
			       TEX_X(x1), TEX_Y(y1), -xdir, -ydir, TEX_LEN(len));
			break;
		    case VECT_END:
			printf("\\put(%d, %d){\\vector(%d,%d){%d}}\n",
			       TEX_X(x0), TEX_Y(y0), xdir, ydir, TEX_LEN(len));
			break;
		    default:
			printf("\\put(%d, %d){\\line(%d, %d){%d}}\n",
			       TEX_X(x0), TEX_Y(y0), xdir, ydir, TEX_LEN(len));
			break;
		}
	}
}
texPicArc(x, y, r, dir, style)
int	x, y, r;
enum	direction dir;
int	style;
{
	char	*dirstr;
	switch (dir) {
	    case dir_br:
		dirstr = "br";
		break;
	    case dir_bl:
		dirstr = "bl";
		break;
	    case dir_tl:
		dirstr = "tl";
		break;
	    case dir_tr:
		dirstr = "tr";
		break;
	}
	texLineWidth(style);
	printf("\\put(%d,%d){\\oval(%d,%d)[%s]}\n", TEX_X(x), TEX_Y(y),
		TEX_LEN(r * 2), TEX_LEN(r * 2), dirstr);
}
texPicText(s, xpos, vpos, factor)
char	*s;
int	xpos;
int	vpos;
int	factor;
{
	printf("\\put(%d,%d){", TEX_X(xpos) + texUnit / 2,
	       TEX_Y(vpos) - texUnit * 3 / 2);
	if (factor * 10 / texUnit > 26)
		printf("\\tiny ");
	else if (factor * 10 / texUnit > 20)
		printf("\\scriptsize ");
	else if (factor * 10 / texUnit > 14)
		printf("\\footnotesize ");
	else if (factor * 10 / texUnit > 10)
		printf("\\small ");
	else
		printf("\\normalsize ");
	printf("%s}\n",codeCvt(texTextQuote(s, 1)));
}
#endif
texTitle(style, haifu, title, shozoku, number, date, name, renraku
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
		return 0;
	}
#ifdef	NEC_CCS
	printf("\\input{a47}\n");
	if (style == TITLE_A47)
		printf("\\Ayonnana");
	else if (style == TITLE_A47_1)
		printf("\\shounin");
	if (*haifu) {
		printf("{");
		for (s = haifu; *s != NULL; s++)
			printf("%s\\\\\n", codeCvt(texTextQuote(*s, 1)));
		printf("}\n");
	}
	else 
		printf("{~}\n");
	printf("{");
	for (s = title; *s != NULL; s++)
		printf("%s\\\\\n", codeCvt(texTextQuote(*s, 1)));
	printf("}\n");
	printf("{");
	for (s = number; *s != NULL; s++)
		printf("%s\n", codeCvt(texTextQuote(*s, 1)));
	printf("}\n");
	printf("{");
	for (s = date; *s != NULL; s++)
		printf("%s\n", codeCvt(texTextQuote(*s, 1)));
	printf("}\n");
	printf("{");
	for (s = shozoku; *s != NULL; s++) {
		printf("%s", codeCvt(texTextQuote(*s, 1)));
		if (*(s + 1) != NULL)
			printf("\\\\\n");
		else
			printf("\n");
	}
	printf("}\n");
	printf("{");
	for (s = name; *s != NULL; s++)
		printf("%s\n", codeCvt(texTextQuote(*s, 1)));
	printf("}\n");
	printf("{");
	for (s = renraku; *s != NULL; s++)
		printf("%s\n", codeCvt(texTextQuote(*s, 1)));
	printf("}\n");
#endif
}
texTable(begin, end, tblp)
int	begin;
int	end;
struct	table	*tblp;
{
	int	j, l, fld;
	char	*atr, *fstr;
	char	format;
	int	hattr;
	int	exCol;
	printf("\\begin{tabular}{");
	for (j = 0; j < tbl_field[0].vlines; j++)
		printf("|");
	for (fld = 1; fld < tbl_nfield; fld++){
		printf("%c", tbl_field[fld].defFormat);
		for (j = 0; j < tbl_field[fld].vlines; j++)
			printf("|");
	}
	printf("}\n");

	for (l = begin; l < end; l++) {
		hattr = lineAttr(texts[l]->body, &atr);
		if (hattr == HORI_DBL_ALL) {
			printf("\\hline\\hline\n");
			continue;
		}
		else if (hattr == HORI_SNGL_ALL) {
			printf("\\hline\n");
			continue;
		}
		if (hattr == HORI_EXIST) {
			int	allHline = 1;
			for (fld = 1; fld < tbl_nfield; fld ++)
				if (attrOfStr(atr, texts[l]->length, fld, 0, tblp)
				    == HORI_SNGL_ALL) 
					printf("\\cline{%d-%d}", fld, fld);
				else if (!fieldEmpty(atr, texts[l]->length, fld, 0, tblp))
					allHline = 0;
			if (allHline)
				continue;
		}
		for (fld = 1; fld < tbl_nfield; fld += exCol + 1) {
			exCol = extraColumn(atr, texts[l]->length, fld, tblp);
			fstr  = codeCvt(texTextQuote(tblSubstr(l, fld, exCol, tblp)
						     ,0));
			format = formatOf(atr, texts[l]->length, fld, exCol, tblp);

			if (attrOfStr(atr, texts[l]->length, fld, exCol, tblp)
			    == HORI_SNGL_ALL)
				/* do nothing	*/;
			else if (exCol ||
				 (format != tbl_field[fld].defFormat
			  && !fieldEmpty(atr, texts[l]->length, fld, exCol, tblp))) {
				printf("\\multicolumn{%d}{", exCol + 1);
				if (tbl_field[fld - 1].vlines)
					printf("|");
				printf("%c", format);
				for (j = 0; j < tbl_field[fld + exCol].vlines; j++)
					printf("|");
				printf("}{%s}", fstr);
			}
			else {
				    printf("%s", fstr);
			}
			if (fld + exCol != tbl_nfield - 1)
				printf(" & ");
		}
		printf("\\\\\n");
	}
	printf("\\end{tabular}\\\\\n");
}
texCapsule(begin, type, center)
int	begin;
int	type;
int	center;
{
	char	*typeStr;
	if (crossRefer) {
		switch (type) {
		    case FT_FIGURE:
			typeStr = "figure";
			break;
		    case FT_TABLE:
			typeStr = "table";
			break;
		    default:
			fprintf(stderr, "PANIC(unknown capsule type)\n");
			exit(2);
			break;
		}
		if (begin) {
			printf("\\begin{%s}[htbp]\n", typeStr);
			if (center) {
				printf("\\begin{center}\n");
			}
		}
		else {
			if (center) {
				printf("\\end{center}\n");
			}
			printf("\\end{%s}\n", typeStr);
		}
	}
	else /* short paper	*/ {
		if (begin) {
			printf("~\\\\\n");
			if (center) {
			 /* 「\begin{center}」の直前が「\\」となるため
			    「\mbox{}」を入れてunderfill hboxの警告を抑える */
				printf("\\mbox{}\\begin{center}\n");
			}
		}
		else {
			if (center) {
				printf("\\end{center}\n");
			}
		}
	}
}
texComment(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end; l++) {
		printf ("%%%% %s\n", codeCvt(texts[l]->body));
	}
}

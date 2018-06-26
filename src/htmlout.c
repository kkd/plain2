/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
/*
    modify from texout.c, by k-chinen@is.aist-nara.ac.jp, 1994

    NOTE:
	* This module is prototype of HTML ouputting.
	* HTML can markup. but HTML cannot layout.
	  Therfore this module cannot handle CENTER, RIGHT and SPACE.
	* HTML cannout handle table and picture directly.
	  Therfore this module cannot handle table and picture with
	  TeX and TROFF outoputting's approach.
	  Then I could not implement it.


    modified by KOBAYASHI Kenichi, 2002

    NOTE:
	* Now, TABLE is rendered in HTML. 
        * Add HTML header.
	* Hyperlink is supported.
	* Footnote is supported.
	* HTML only macro ((mh)) is supported.
	* Crossreference is enhanced.
	* Fix table/figure captions. (but needs more enhancement...)
	* Tex-like superscribe and subscribe are supported (($)).
	* Trim newlines and spaces.
	* Keep completely backward compatibility with -htmlold option.
*/

#ifdef HTML


#ifndef lint


#endif

#include <stdio.h>
#include "plain2.h"
#include "picture.h"
#include "table.h"
#include "macro.h"


struct	macDefs htmlMacros[] = {
/* KK (KOBAYASHI Kenichi) */
	M_DOC_BEGIN,	"<HTML>\n<HEAD>\n",
	M_DOC_END,	"</BODY>\n</HTML>\n",
	M_PLAIN_BEGIN,	"<P>\n",
	M_PLAIN_END,	"</P>\n",
	M_EXAM_BEGIN,	"<PRE>\n",
	M_EXAM_END,	"</PRE>\n",
	M_APDX_BEGIN,	"<!-- appendix -->\n",
	M_APPENDIX,	"\n<H2><A name=\"@1\">@1</A></H2>\n",
	M_BLANK,	"<BR>\n",
	M_PAGE,		"\n<HR>\n",
	M_NEWLINE,	"<BR>\n",
	M_CENTER_BEGIN,	"<DIV align=\"center\">\n",
	M_CENTER_END,	"</DIV>\n",
	M_RIGHT_BEGIN,	"<DIV align=\"right\">\n",
	M_RIGHT_END,	"</DIV>\n",
	M_INDENT,	"<BLOCKQUOTE>\n",
	M_INDENT0,	"</BLOCKQUOTE>\n",
	M_FOOTN_BEGIN,	"<SMALL>",
	M_FOOTN_END,	"</SMALL>",
	M_REFER_BEGIN,	"<A href=\"#",
	M_REFER_END,	"\">",
	M_BOLD_BEGIN,	"<B>",
	M_BOLD_END,	"</B>",
	M_INDEX_BEGIN,	"<A name=\"",
	M_INDEX_END,	"\">",
	M_SECTION_1,	"\n<H2><A name=\"@1\">@1</A></H2>\n",
	M_SECTION_2,	"\n<H3><A name=\"@1\">@1</A></H3>\n",
	M_SECTION_3,	"\n<H4>@1</H4>\n",
	M_SECTION_4,	"\n<H5>@1</H5>\n",
	M_SECTION_5,	"\n<H6>@1</H6>\n",
	M_SECTION,	"",
	M_SETSEC_1,	"",
	M_SETSEC_2,	"",
	M_SETSEC_3,	"",
	M_SETSEC_4,	"",
	M_SETSEC_5,	"",
	M_TITLE, "<H1>&1</H1>\n<TABLE width=\"100%\">\n<TR><TD>&3</TD><TD align=\"right\">&2</TD></TR>\n<TR><TD>&4 &6</TD><TD align=\"right\">&5</TD></TR>\n<TR><TD>&7</TD></TR>\n</TABLE>\n<HR>\n",
	-1,	"",
	};

struct	macDefs htmlOldMacros[] = {
/* KK end */
	M_DOC_BEGIN,	"<HTML>\n",
	M_DOC_END,	"</HTML>\n",
	M_PLAIN_BEGIN,	"<P>\n",
	M_PLAIN_END,	"</P>\n",
	M_EXAM_BEGIN,	"<PRE><TT>\n",
	M_EXAM_END,	"</TT></PRE>\n",
	M_APDX_BEGIN,	"<!-- appendix -->\n",
	M_APPENDIX,	"<!-- appendix section -->\n",
	M_BLANK,	"\n",
	M_PAGE,		"\n<!-- PAGE -->\n",
	M_NEWLINE,	"\n<!-- NEWLINE -->\n",
	M_CENTER_BEGIN,	"<!-- CENTER -->\n",
	M_CENTER_END,	"<!-- end of CENTER -->\n",
	M_RIGHT_BEGIN,	"<!-- RIGHT -->\n",
	M_RIGHT_END,	"<!-- end of RIGHT -->\n",
	M_INDENT,	"<BLOCKQUOTE>\n",
	M_INDENT0,	"</BLOCKQUOTE>\n",
	M_FOOTN_BEGIN,	"",
	M_FOOTN_END,	"",
	M_REFER_BEGIN,	"<A HREF=\"#",
	M_REFER_END,	"\">GO</A>",
	M_BOLD_BEGIN,	"<B>",
	M_BOLD_END,	"</B>",
	M_INDEX_BEGIN,	"",
	M_INDEX_END,	"",
	M_SECTION_1,	"\n<H1>@1</H1>\n",
	M_SECTION_2,	"\n<H2>@1</H2>\n",
	M_SECTION_3,	"\n<H3>@1</H3>\n",
	M_SECTION_4,	"\n<H4>@1</H4>\n",
	M_SECTION_5,	"\n<H5>@1</H5>\n",
	M_SECTION,	"",
	M_SETSEC_1,	"",
	M_SETSEC_2,	"",
	M_SETSEC_3,	"",
	M_SETSEC_4,	"",
	M_SETSEC_5,	"",
	M_TITLE, "<TITLE>&1</TITLE>\n<H1>&1</H1>\n<EM>&6</EM>\n<PRE>&4\n&3</PRE>\n<HR>",
	-1,	"",
	};


/*
 * HTML text output routines
 */ 

struct transTable	*htmlTrans;
struct transTable	*htmlTransM;

static struct transTable	htmlFullTrans[] = {
	{'<', "&lt;" },
	{'>', "&gt;" },
	{'&', "&amp;" },
	{0,0} };
static struct transTable	htmlFullTransM[] = {
	{'<', "&lt;" },
	{'>', "&gt;" },
	{'&', "&amp;" },
	{0,0} };

static struct transTable	htmlHalfTrans[] = {
	{'<', "&lt;" },
	{'>', "&gt;" },
	{'&', "&amp;" },
	{0,0} };

static struct transTable	htmlHalfTransM[] = {
	{'<', "&lt;" },
	{'>', "&gt;" },
	{'&', "&amp;" },
	{0,0} };


/* KK */
int	htmlFootnoteHere = 1;
static int paragraph_is_opend = 0;
static int suppress_blank = 0;
static int suppress_paragraph = 0;
static int suppress_paragraph_but_newline = 0;

char *strsave();
char *getTitle();
char *getAuthor();
char *getNthLine();
/* KK end */


htmlSetTrans(full)
int	full;
{
	if (full) {
		htmlTrans  = htmlFullTrans;
		htmlTransM = htmlFullTransM;
	}
	else {
		htmlTrans  = htmlHalfTrans;
		htmlTransM = htmlHalfTransM;
	}
}


char	*
htmlTextQuote(str, quotable)
char	*str;
int	quotable;
{
	/* KK */
	if (rawOutput)
        	return str;

    	if (!quotable)
        	return textQuote(str, htmlTransM);

       	return textQuote(str, htmlTrans);
}

char	*htmlQuote1(str)
char	*str;
{
	return htmlTextQuote(str, 1);
}

char	*htmlQuote2(str)
char	*str;
{
	return htmlTextQuote(str, 0);
}


/* KK */
htmlPutLabel(str, begin)
char	*str;
int	begin;
{
	if (begin) {
		printf("<A name=\"%s\">", codeCvt(str));
	} else {
		printf("</A>");
	}
}

htmlPlain(str, attr, newline)
char	*str;
int	attr;
int	newline;
{
	static int offset = 0;
	char buf[MAX_LINE_LEN];
	int suppress_newline = 0;
	char *quoted;
	int n;

	if (attr == IL_RAW || attr == IL_REFERENCE)
		printf("%s", codeCvt(str));
	else {
		/* To avoid newline between zenkaku chars */
		quoted = htmlTextQuote(str, 1);
		if (isHankakuAllStr(quoted)) {
			printf("%s", codeCvt(quoted));
		} else {
			n = searchStrBreak(quoted, 60 - offset, 78 - offset);
			if (n == 0) {
				printf("%s", codeCvt(quoted));
			} else {
				strncpy(buf, quoted, n);
				buf[n] = '\0';
				printf("%s\n", codeCvt(buf));
				strncpy(buf, quoted + n, sizeof(buf));
				buf[sizeof(buf)-1] = '\0';
				printf("%s", codeCvt(buf));
			}
			offset = strlen(quoted) - n;
			suppress_newline = 1;
		}
	} 
	if (newline && !suppress_newline) {
		printf("\n");
		offset = 0;
	}
}
/* KK end */

htmlExample(str)
char	*str;
{
/* KK */
	if (htmlOld) {
		printf("%s\n",textQuote(str, htmlTrans));
	} else {
		printf("%s\n",codeCvt(textQuote(str, htmlTrans)));
	}
/* KK end */
}

static int	enumLevel = 0;
htmlListBlock(begin, level, ltype, hint)
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
			printf("<UL>\n");
			break;
		    case L_DLIST:
			printf("<DL>\n");
			break;
		    case L_LROMAN:
		    case L_SROMAN:
		    case L_NUMBER:
		    case L_LALPHA:
		    case L_SALPHA:
			enumLevel++;
			printf("<OL>\n");
			break;
		    default:
			break;
		}
	}
	else {
		switch (ltype) {
		    case L_BULLET:
		    case L_DASH:
			printf("</UL>\n");
			break;
		    case L_LROMAN:
		    case L_SROMAN:
		    case L_NUMBER:
		    case L_LALPHA:
		    case L_SALPHA:
			enumLevel--;
			printf("</OL>\n");
			break;
		    case L_DLIST:
			printf("</DL>\n");
			break;
		    default:
			break;
		}
	}
}

htmlDlistItem(level, dscr, cont)
int	level;
char	*dscr;
int	cont;
{
	printf("<DT><B>%s</B>\n", codeCvt(htmlTextQuote(dscr, 1)));
}

htmlListItem(level)
int	level;
{
	printf("<LI> ");
}

htmlRawText(str)
char	*str;
{
	printf("%s\n",codeCvt(str));
}

htmlSpace(length)
int	length;	
{
	/* EMPTY --- I don't know what do here */
}

htmlFTitle(ft, str, capsule)
int	ft;
char	*str;		/* if (str!=NULL) output title	*/
int	capsule;
{
	static int	figNumber = 1;
	static int	tblNumber = 1;
/* KK */
	if (crossRefer && capsule) {
		if (!htmlOld) {
			htmlPutLabel(str, BEGIN);
		} else {
			printf("<B>%s</B>\n", codeCvt(htmlTextQuote(str, 1)));
			printf("<A NAME=\"%s\"> <EM>(here)</EM> </A>",
			       codeCvt(str));
		}
	} else {
		printf ("<P>");
	}
	if (!htmlOld || !(crossRefer && capsule)) {
/* KK end */
#ifdef	KANJI
		if (japaneseText) {
			if (ft == FT_FIGURE)
				printf ("<B> %s %d.</B>", codeCvt("¿Þ"), figNumber++);

			else if (ft == FT_TABLE)
 				printf ("<B> %s %d.</B>", codeCvt("É½"), tblNumber++);
		}
		else {
#endif
			if (ft == FT_FIGURE)
				printf ("<B> Figure %d.</B>", figNumber++);
			else if (ft == FT_TABLE)
				printf ("<B> Table %d.</B>", tblNumber++);
#ifdef	KANJI
		}
#endif
		printf ("%s\n", codeCvt(htmlTextQuote(str, 1)));
/* KK */
		if (crossRefer && capsule) {
			htmlPutLabel(str, END);
		}
/* KK end */
	}
}

#ifdef	PICTURE
/*
 * Driver for HTML(picture) output
 */
static int	picLines;		/* Number of lines in the block	*/
static int	picMinInd;		/* Indentation of the region	*/

static int	htmlVsize;
static int	htmlUnit;

htmlPictureBlock(begin, lines, indent, maxLen)
int	begin;
int	lines;
int	indent;
int	maxLen;
{
	if(htmlOnce) {
	    if(begin)
		    printf("<!-- PICTURE -->\n");
	}
	else {
	    if(begin)
		    printf("<!-- PICTURE -->\n");
	}
}

/* set line width	*/
htmlLineWidth(style)
int	style;
{
	/* */
}

htmlEllipse(x0, y0, x1, y1, style)
int	x0, x1, y0, y1;
int	style;
{
	/* */
}

/*
 * Draw line
 *	Only ({-1,0,1},{-1,0,1}) direction.
 */
htmlPicLine(x0, y0, x1, y1, style, vector)
int	x0, x1, y0, y1;
int	style;
int	vector;
{
	/* */
}

htmlPicArc(x, y, r, dir, style)
int	x, y, r;
enum	direction dir;
int	style;
{
	/* */
}

htmlPicText(s, xpos, vpos, factor)
char	*s;
int	xpos;
int	vpos;
int	factor;
{
	/* */
}
#endif



htmlTitle(style, haifu, title, shozoku, number, date, name, renraku
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
/* KK */
		htmlSuppressBlank(99999999);
		printf("\n<!-- toc -->\n");
/* KK end */
		return 0;
	}
}


/* KK */
/* === HTML table functions ======================================== */

struct table_row_info {
	int  	attr;
	int	disable;
};


struct table_field_info {
	int	td_tag;
	char	*align;
	int	rowspan;
	int	colspan;
	int	disable;
	int	origin;
};


static int
isThereAnyString(hattr, atr, l, tblp)
int	hattr;
char	*atr;
int	l;
struct	table	*tblp;
{
	int 	fld;

	if (hattr == HORI_DBL_ALL || hattr == HORI_SNGL_ALL) {
		return 0;
	} else if (hattr == HORI_EXIST) {
		for (fld = 1; fld < tbl_nfield; fld ++) {
			if (attrOfStr(atr, texts[l]->length, fld, 0, tblp)
			    != HORI_SNGL_ALL
/*			    &&
			    !fieldEmpty(atr, texts[l]->length, fld, 0, tblp)*/
			    ) {
				return 1;
			}
		}
	} else {
		return 1;
	}
	return 0;
}


static int
calcExcol(l, fld, tblp)
int	l;
int	fld;
struct	table	*tblp;
{
	int	exCol;
	char	*atr;

	lineAttr(texts[l]->body, &atr);
	exCol = extraColumn(atr, texts[l]->length, fld, tblp);
	return exCol;
}


static char*
getField(l, fld, tblp)
int	l;
int	fld;
struct	table	*tblp;
{
	int	hattr, exCol;
	char	*atr, *fstr;

	hattr = lineAttr(texts[l]->body, &atr);
	if (hattr == HORI_SNGL_ALL || hattr == HORI_DBL_ALL) {
		return 0;
	}
	exCol = extraColumn(atr, texts[l]->length, fld, tblp);
	fstr = codeCvt(htmlTextQuote(tblSubstr(l, fld, exCol, tblp), 0));
	if (attrOfStr(atr, texts[l]->length, fld, exCol, tblp)
	    == HORI_SNGL_ALL) {
		return 0;
	}
	return fstr;
}


static
createTableInfo(begin, end, tblp, rowsp, fieldsp, is_borderedp)
int	begin;
int	end;
struct	table	*tblp;
struct 	table_row_info		**rowsp;
struct	table_field_info	**fieldsp;
int	*is_borderedp;
{
	int 	j, l, fld, hattr;
	int	exCol;
	char	*atr;
	struct 	table_row_info		*rows;
	struct	table_field_info	*fields;
	struct	table_field_info	*fldp;
	char	format;
	int n_dbl_hline = 0;
	int is_bordered = 0;
	int first_real_row = -1;
	int last_real_row = -1;

	rows = (struct table_row_info*)
		(malloc(sizeof(struct table_row_info) * (end - begin)));
	if (rows == NULL) {
		fprintf(stderr, "PANIC(malloc in createTableInfo)\n");
		exit (2);
	}
	fields = (struct table_field_info*)
		malloc((end - begin) * tbl_nfield
		       * sizeof(struct table_field_info));
	if (fields == NULL) {
		fprintf(stderr, "PANIC(malloc in createTableInfo)\n");
		exit (2);
	}
	*rowsp = rows;
	*fieldsp = fields;
	for (fld = 0; fld < tbl_nfield; fld++){
		if (tbl_field[fld].vlines > 0) {
			is_bordered = 1;
			break;
		}
	}
	for (l = begin; l < end; l++) {
		hattr = lineAttr(texts[l]->body, &atr);
		if (hattr == HORI_SNGL_ALL 
		    || hattr == HORI_DBL_ALL
		    || hattr == HORI_EXIST) {
			is_bordered = 1;
		}
		if (hattr == HORI_DBL_ALL) {
			n_dbl_hline++;
		} else if (hattr == HORI_SNGL_ALL
			   && l > begin
			   && rows[l - 1 - begin].attr == HORI_SNGL_ALL) {
			n_dbl_hline++;
			rows[l - 1 - begin].attr = HORI_DBL_ALL;
		}
		rows[l - begin].attr = hattr;
		rows[l - begin].disable
			= !isThereAnyString(hattr, atr, l, tblp);
		if (!rows[l - begin].disable) {
			if (first_real_row == -1) first_real_row = l - begin;
		}
	}
	for (l = begin; l < end; l++) {
		lineAttr(texts[l]->body, &atr); /* don't remove */
		hattr = rows[l - begin].attr;
		if (!rows[l - begin].disable) last_real_row = l - begin;
		exCol = 0;
		for (fld = 1; fld < tbl_nfield; fld++) {
			fldp = &fields[(l - begin) * tbl_nfield + fld];
			if (exCol == 0) {
				exCol = extraColumn(atr, texts[l]->length,
						    fld, tblp);
				fldp->disable = 0;
				fldp->colspan = 1 + exCol;
			} else {
				fldp->disable = 1;
				fldp->colspan = 1;
				exCol--;
			}
			fldp->td_tag = 1;
			fldp->rowspan = 1;
			format = formatOf(atr, texts[l]->length,
					  fld, exCol, tblp);
			fldp->align =
				(format == 'c' ? " align=\"center\"" :
				 format == 'r' ? " align=\"right\"" :
				 " align=\"left\"");
			if (attrOfStr(atr, texts[l]->length, fld, exCol, tblp)
			    == HORI_SNGL_ALL) {
				fldp->disable = 1;
			}
			fldp->origin = -1;
			
			if (hattr == HORI_DBL_ALL
			    && n_dbl_hline <= 1
			    && last_real_row >= 0
			    && last_real_row == first_real_row) {
				fields[last_real_row * tbl_nfield + fld]
					.td_tag = 0;
				rows[l - begin].attr = HORI_SNGL_ALL;
			}
		}
	}
	if (is_borderedp != NULL) {
		*is_borderedp = is_bordered;
	}

	if (!is_bordered) return 0;

	/* process rowspan */
	for (l = begin; l < end; l++) {
		if (rows[l - begin].disable) continue;
		for (fld = 1; fld < tbl_nfield; fld += 1 + exCol) {
			fldp = &fields[(l - begin) * tbl_nfield + fld];
			exCol = fldp->colspan - 1;
			if (fldp->disable) continue;
			for (j = l + 1; j < end; j++) {
				int k;
				if (calcExcol(j, fld, tblp) != exCol) break;
				for (k = 0; k <= exCol; k++) {
					if (getField(j, fld + k, tblp) == 0) {
						break;
					}
				}
				if (k <= exCol) break;
				fldp->rowspan++;
				for (k = 0; k <= exCol; k++) {
					fields[(j - begin) * tbl_nfield + fld + k].disable
						= 1;
					fields[(j - begin) * tbl_nfield + fld + k].origin
						= l;
				}
			}
		}
	}

	/* shrink rows */
	for (l = begin; l < end; l++) {
		int disable = 1;
		if (rows[l - begin].disable) continue;
		for (fld = 1; fld < tbl_nfield; fld += 1 + exCol) {
			fldp = &fields[(l - begin) * tbl_nfield + fld];
			exCol = fldp->colspan - 1;
			if (fldp->disable) continue;
			disable = 0;
		}
		if (!disable) continue;
		rows[l - begin].disable = 1;
		for (fld = 1; fld < tbl_nfield; fld += 1 + exCol) {
			fldp = &fields[(l - begin) * tbl_nfield + fld];
			exCol = fldp->colspan - 1;
			if (fldp->origin >= 0
			    && fields[(fldp->origin - begin) * tbl_nfield
				      + fld].rowspan > 1) {
				fields[(fldp->origin - begin) * tbl_nfield
				       + fld].rowspan--;
			}
		}
	}
}


static
htmlEmbededTable(begin, end, tblp)
int	begin;
int	end;
struct	table	*tblp;
{
	int	j, l, fld;
	char	*fstr;
	int	exCol, td_tag, colspan;
	int	is_bordered;
	struct table_row_info	*rows;
	struct table_field_info	*fields, *fldp, *fldp2;
	static int tbl_count = 0;
	int n_dbl_vline = 0;
	int vlines;
	int vth = 0;

	createTableInfo(begin, end, tblp, &rows, &fields, &is_bordered);
	for (fld = 0; fld < tbl_nfield; fld++){
		if (tbl_field[fld].vlines > 1) {
			n_dbl_vline++;
		}
	}
	if (n_dbl_vline == 1 && tbl_field[1].vlines > 1) {
		vth = 1;
		tbl_field[1].vlines = 1;
	}
	printf("<TABLE%s id=\"tbl:%d\">\n",
	       is_bordered ? " border=\"2\"" : "", tbl_count++);
	for (l = begin; l < end; l++) {
		if (rows[l - begin].disable) {
			if (rows[l - begin].attr == HORI_DBL_ALL) {
				printf("  <TR></TR>\n");
			}
			continue;
		}
		printf("  <TR>");
		for (fld = 1; fld < tbl_nfield; fld += exCol + 1) {
			fldp = &fields[(l - begin) * tbl_nfield + fld];
			exCol = fldp->colspan - 1;
			if (fldp->disable) {
				if (tbl_field[fld + exCol].vlines > 1) {
					printf("<TD></TD>");
				}
				continue;
			}
			fstr  = codeCvt(
				htmlTextQuote(
					tblSubstr(l, fld, exCol, tblp) ,0));
			vlines = 0;
			for (j = 0; j < exCol; j++) {
				if (tbl_field[fld + j].vlines > 1) {
					vlines++;
				}
			}
			td_tag = fldp->td_tag;
			if (fld == 1 && exCol == 0 && vth) {
				td_tag = 0;
			}
			printf("<%s%s", (td_tag ? "TD" : "TH"), fldp->align);
			if (fldp->colspan + vlines> 1) {
				printf(" colspan=\"%d\"",
				       fldp->colspan + vlines);
			}
			if (fldp->rowspan > 1) {
				printf(" rowspan=\"%d\"", fldp->rowspan);
			}
			printf(">%s", fstr);
			fldp2 = fldp;
			for (j = l + 1; j < end; j++) {
				fldp2 += tbl_nfield;
				if (fldp2->origin != l) break;
				fstr  = codeCvt(
					htmlTextQuote(
						tblSubstr(j, fld,
							  exCol, tblp) ,0));
				printf("<BR>\n%s", fstr);
			}
			{
			    char *p;
			    for (p = fstr; *p == ' '; p++);
			    if (*p == '\0') {
				printf("&nbsp;");
			    }
			}
			printf("</%s>", (td_tag ? "TD" : "TH"));

			if (tbl_field[fld + exCol].vlines > 1) {
				printf("<TD></TD>");
			}
		}
		printf("</TR>\n");
	}
	printf("</TABLE>\n");
	free(rows);
	free(fields);
}
/* KK end */


htmlTable(begin, end, tblp)
int	begin;
int	end;
struct	table	*tblp;
{
/* KK */
	if (!htmlOld) {
		htmlEmbededTable(begin, end, tblp);
		return 0;
	}
/* KK end */
	if(htmlOnce) {
		if(rawOutput) {
			int	l;

			printf("<PRE>\n");
			for (l = begin; l < end; l++) {
				htmlRawText(texts[l]->body);
			}
			printf("</PRE>\n");
		}
		else {
			printf("<!-- TABLE -->\n");
		}

	}
	else {
	    static int tbl_count=0;
	    printf("\n<!-- plain2:TABLE %05d %d %d -->\n",tbl_count,begin,end);

	    if(htmlHere) {
		printf("<P><IMG SRC=\"TBL%05d.gif\">\n", tbl_count);
	    }
	    else {
		printf("<P><B><A HREF=\"TBL%05d.gif\">Table here</A></B></P>\n",
		    tbl_count);
	    }
	    tbl_count++;
	}
}

htmlCapsule(begin, type, center)
int	begin;
int	type;
int	center;
{
	static int count=0;
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
			count++;
			printf("\n<!-- Start of #%d -->\n",count);
		}
		else {
			printf("\n<!-- End of #%d -->\n",count);
		}
	}
	else /* short paper	*/ {
		if (begin) {
			printf("\n<!-- Start of #%d -->\n",count);
		}
		else {
			printf("\n<!-- End of #%d -->\n",count);
		}
	}
}

htmlComment(begin, end)
int	begin;
int	end;
{
/* KK */
	char	buf[MAX_LINE_LEN];
	int	l, ch, i;
	char	*p;

	for (l = begin; l < end; l++) {
		ch = 0;
		i = 0;
		for (p = texts[l]->body; *p; p++) {
			if (i > sizeof(buf) - 2) break;
			if (ch == '-' && *p == '-') {
				buf[i++] = ' ';
			}
			ch = buf[i++] = *p;
		}
		buf[i] = '\0';
		printf ("<!-- %s -->\n", codeCvt(buf));
	}
/* KK end */
}


/* KK */
htmlHeader()
{
	extern char* texStyle;

	putMacro(M_DOC_BEGIN, 0, "", "");

	if (outputCode == CODE_EUC ||
	    outputCode == CODE_JIS ||
	    outputCode == CODE_SJIS) {
		    printf("  <META http-equiv=\"Content-Type\"");
		    printf(" content=\"text/html; charset=%s\">\n",
			   outputCode == CODE_EUC ? "EUC-JP" :
			   outputCode == CODE_SJIS ? "Shift_JIS" :
			   "ISO-2022-JP");
	}
	if (getTitle()) {
		printf("  <TITLE>%s</TITLE>\n",
		       codeCvt(htmlQuote1(getTitle())));
	}
	if (getAuthor()) {
		printf("  <META name=\"Author\" content=\"%s\">\n",
		       codeCvt(htmlQuote1(getAuthor())));
	}
	if (strcmp(texStyle, DEFAULT_STY) != 0) {
		printf("  <LINK rel=\"StyleSheet\" type=\"text/css\" href=\"%s\">\n",
		       texStyle);
	}
	printf("</HEAD>\n<BODY>\n");
}


static int footnote_count = 0;
static int footnote_num = 0;
static char** footnote_text_table = NULL;
static int footnote_table_size = 0;

htmlAddFootnote(char *text)
{
	char **new_table;
	int i;
	
	if (footnote_num >= footnote_table_size) {
		footnote_table_size += 128;
		new_table = (char**)malloc(sizeof(char*) * footnote_table_size);
		if (new_table == NULL) {
			fprintf(stderr, "PANIC(malloc in htmlAddFootnote)\n");
			exit (2);
		}
		for (i = 0; i < footnote_table_size - 128; i++) {
			new_table[i] = footnote_text_table[i];
		}
		for (; i < footnote_table_size; i++) {
			new_table[i] = NULL;
		}
		free(footnote_text_table);
		footnote_text_table = new_table;
	}
	footnote_text_table[footnote_num] = strsave(text);
	footnote_num++;
	footnote_count++;
	if (!htmlFootnoteHere) {
		printf("<SUP><A href=\"#footnote:%d\" name=\"footnotemark:%d\">*%d</A></SUP>\n",
		       footnote_count, footnote_count, footnote_count);
	} else {
		printf("<SUP>*%d</SUP>\n", footnote_count);
	}
}

/*
htmlExtendFootnote(char *text)
{
	char *p;
	int len;

	if (footnote_count == 0) return 0;
	len = strlen(footnote_text_table[footnote_count - 1])
		+ strlen(text);
	p = malloc(len + 1);
	if (p == NULL) {
		fprintf(stderr, "PANIC(malloc in htmlExtendFootnote)\n");
		exit (2);
	}
	strcpy(p, footnote_text_table[footnote_count - 1]);
	strcat(p, text);
	free(footnote_text_table[footnote_count - 1]);
	footnote_text_table[footnote_count - 1] = p;
}
*/

htmlPutFootnote(int finish)
{
    	int i, j, n;
	int cnt;

	if (footnote_num > 0) {
		if (!htmlFootnoteHere) {
			printf("<HR>\n");
		} else {
			printf("<BLOCKQUOTE>\n<HR>\n");
		}
	}
	for (i = 0; i < footnote_num; i++) {
		cnt = footnote_count - footnote_num + i + 1;
		if (!htmlFootnoteHere) {
			printf("<SUP><A href=\"#footnotemark:%d\" name=\"footnote:%d\">*%d</A></SUP>\n",
			       cnt, cnt, cnt);
		} else {
			printf("<SUP>*%d</SUP>\n", cnt);
			
		}
		putMacro(M_FOOTN_BEGIN);
		n = getNumLines(footnote_text_table[i]);
		for (j = 1; j <= n; j++) {
			htmlPlain(getNthLine(footnote_text_table[i], j),
				  IL_FOOTNOTE, 0);
		}
		putMacro(M_FOOTN_END);
		free(footnote_text_table[i]);
		footnote_text_table[i] = NULL;
		printf("<BR>\n");
	}
	if (footnote_num > 0 && htmlFootnoteHere) {
		printf("<HR>\n</BLOCKQUOTE>\n");
	}
	footnote_num = 0;
	if (finish) {
		footnote_count = 0;
	}
}

htmlSuppressBlank(l)
int	l;
{
	suppress_blank = l;
}

htmlSuppressParagraphButNewline(l)
int	l;
{
	suppress_paragraph_but_newline = l;
}

htmlSuppressParagraph(l)
int	l;
{
	suppress_paragraph = l;
}

htmlMacroPlainBegin(begin, end)
int	begin;
{
	if (suppress_paragraph < begin) {
		if (suppress_paragraph_but_newline < begin) {
			putMacro(M_PLAIN_BEGIN);
			paragraph_is_opend = 1;
		} else {
			putMacro(M_NEWLINE);
			suppress_paragraph_but_newline = 0;
		}
	} else {
		suppress_paragraph = 0;
	}
	suppress_blank = 0;
}

htmlMacroPlainEnd(end)
int	end;
{
	if (paragraph_is_opend) {
		putMacro(M_PLAIN_END);
		paragraph_is_opend = 0;
		suppress_blank = end;
	} else {
		putMacro(M_NEWLINE);
		suppress_blank = 0;
	}
}

htmlMacroBlank(l)
int	l;
{
	if (suppress_blank < l) {
		putMacro(M_BLANK);
	} else {
		suppress_blank = 0;
	}
}

htmlCloseReference()
{
	printf("</A>\n");
}

htmlOpenHyperlink(link)
char	*link;
{
	printf("<A href=\"");
	htmlPlain(link, IL_RAW, 0);
	printf("\">");
}

htmlCloseHyperlink()
{
	printf("</A>\n");
}

htmlCloseDTerm()
{
	printf("<DD>");
}

/* KK end */

#endif /* HTML */

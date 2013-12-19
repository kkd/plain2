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
*/




#ifndef lint


#endif

#include <stdio.h>
#include "plain2.h"
#include "picture.h"
#include "table.h"
#include "macro.h"


struct	macDefs htmlMacros[] = {
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
	char	*s;
	int	len;
	static	char	buf[MAX_LINE_LEN];

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


htmlPutLabel(str)
char	*str;
{
	printf("<A NAME=\"%s\"> <EM>(here)</EM> </A>", codeCvt(str));
}

htmlPlain(str, attr, newline)
char	*str;
int	attr;
int	newline;
{
	if (attr == IL_RAW || attr == IL_REFERENCE)
		printf("%s", codeCvt(str));
	else
		printf("%s", codeCvt(htmlTextQuote(str, 1)));
	if (newline)
		printf("\n");
}

htmlExample(str)
char	*str;
{
	printf("%s\n",textQuote(str, htmlTrans));
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
	if (crossRefer && capsule) {
		printf("<B>%s</B>\n", codeCvt(htmlTextQuote(str, 1)));
		htmlPutLabel(str);
	}
	else {
		printf ("<P>");
#ifdef	KANJI
		if (japaneseText) {
			if (ft == FT_FIGURE)
				printf ("<B> %s %d.</B>", codeCvt("図"), figNumber++);

			else if (ft == FT_TABLE)
 				printf ("<B> %s %d.</B>", codeCvt("表"), tblNumber++);
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
		return;
	}
}

htmlTable(begin, end, tblp)
int	begin;
int	end;
struct	table	*tblp;
{
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
	int	l;
	for (l = begin; l < end; l++) {
		printf ("<!-- %s -->\n", codeCvt(texts[l]->body));
	}
}

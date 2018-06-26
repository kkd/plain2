/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 */
/*
 * Table parsing routine
 */
#ifndef lint
static char rcsid[] =
	"$Id: table.c,v 2.13 1994/04/19 10:17:06 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
#include "table.h"
#include "kanji.h"

#define	ATR_VLINE	0x01		/* Vertical line	*/
#define	ATR_HLINE	0x02		/* Horizontal line	*/
#define	ATR_HDLINE	0x04		/* Horizontal double	*/
#define	ATR_ANYLINE	(ATR_VLINE | ATR_HLINE | ATR_HDLINE)
#define	ATR_CHAR	0x08	/* non line character		*/
#define	ATR_JIS2	0x10	/* second byte of JIS wakusen	*/
#define	ATR_PLUS	(ATR_HLINE | ATR_HDLINE)
/*
 * Alphabetic frame lines
 */
struct	strVal	tblwaku1[] = {
	{"-",  ATR_HLINE},
	{"=",  ATR_HDLINE},
	{"|",  ATR_VLINE},
	{"+",  ATR_PLUS},
	{"", 0}};
#ifdef	KANJI
/*
 * JIS Keisen
 */
struct	strVal	tblwaku2[] = {
	{"иб", ATR_HLINE},
	{"ив", ATR_VLINE},
	{"иг", ATR_HLINE | ATR_VLINE},
	{"ид", ATR_HLINE | ATR_VLINE},
	{"иж", ATR_HLINE | ATR_VLINE},
	{"ие", ATR_HLINE | ATR_VLINE},
	{"из", ATR_HLINE | ATR_VLINE},
	{"ий", ATR_HLINE | ATR_VLINE},
	{"ии", ATR_HLINE | ATR_VLINE},
	{"ик", ATR_HLINE | ATR_VLINE},
	{"ил", ATR_HLINE | ATR_VLINE},
	{"", 0}};
#endif
#define	COLV_JIS	2
#define	COLV_ASC	1
static	char	colVline[MAX_LINE_LEN];	/* Vertical line exist on the column*/
static	char	colNchrs[MAX_LINE_LEN];	/* Number of chars on the column */
static	char	colPchrs[MAX_LINE_LEN];	/* chars to be printed		*/
static	short	maxLen;			/* Max length of the region	*/
static	short	wakuAll,	wakuExist;
static	short	lineHoriFull,	lineHoriPart;

int	expectedFields;		/* Number of expected fields	*/

static	unsigned char tblKanjiByte[256];
static	unsigned char tblAsciAttr[256];
tblParseInit()
{
	struct strVal	*twp;
	bzero((char *)tblKanjiByte, sizeof(tblKanjiByte));
	bzero((char *)tblAsciAttr, sizeof(tblAsciAttr));
	for (twp = tblwaku1; *twp->pattern; twp++)
		tblAsciAttr[(unsigned char)*twp->pattern] = twp->value;
#ifdef	KANJI
	for (twp = tblwaku2; *twp->pattern; twp++)
		tblKanjiByte[(unsigned char)*twp->pattern]++;
#endif
}
tblKwakuAttr(s)
register char	*s;
{
	register struct	strVal	*twp;
#ifdef	KANJI
	if (jisTableEnabled && maybeThinKeisen(s)) {
		/* first byte of ибивигидижиеизийииикил	*/
		twp = tblwaku2;
		while (*twp->pattern) {
			if (str2match(s, twp->pattern))
				return twp->value;
			twp++;
		}
	}
#endif
	return 0;
}
/*
 * Attribute of the line (Horizontal line, Double line)
 */
lineAttr(s, horiAtrp)
register char	*s;
char	**horiAtrp;
{
	static	char	attribute[MAX_LINE_LEN];
	register char	*atrp;
	int	charAttr;
	int	twinLine, singleLine;
	int	hexist;
	int	inLeadingSpace = 1;
	if (*s == '\0')
		return	HORI_NULL;
	twinLine = singleLine = 1;
	hexist	 = 0;
	for (atrp = attribute; *s; s++) {
		if ((charAttr = tblAsciAttr[(unsigned char)*s])
		    || (tblKanjiByte[(unsigned char)*s]
			&& (charAttr = tblKwakuAttr(s)))) {
			inLeadingSpace = 0;
			*atrp++ = charAttr;
			if ((charAttr & ATR_VLINE) == 0) {
				twinLine   &= ((charAttr & ATR_HDLINE) != 0);
				singleLine &= ((charAttr & ATR_HLINE) != 0);
				hexist	   |= ((charAttr & ATR_HLINE) != 0);
			}
#ifdef	KANJI
			if (isZenkaku(s)) {
				if (charAttr & ATR_VLINE)
					*atrp = ATR_JIS2;
				else
					*atrp = 0;
				*atrp++ |= (charAttr & ATR_HLINE);
				s++;
			}
#endif
		}
		else if (*s != ' ') {
			inLeadingSpace = 0;
			*atrp++ = ATR_CHAR;
			twinLine = singleLine = 0;
#ifdef	KANJI
			if (isZenkaku(s)) {
				*atrp++ = ATR_CHAR;
				s++;
			}
#endif
		}
		else {
			if (!inLeadingSpace)
				twinLine = singleLine = 0;
			*atrp++ = 0;
		}
	}
	*horiAtrp = attribute;
	return twinLine ? HORI_DBL_ALL
		: (singleLine ? HORI_SNGL_ALL
		   : (hexist ? HORI_EXIST
		      : HORI_NULL)) ;
}
findFieldLeft(col)
int	col;
{
	if (wakuExist) {
		int	seeVline = 0;
		while (col < maxLen) {
			if (colVline[col] == 0 && colNchrs[col])
				return col;
			if (colVline[col] == 0 && seeVline)
				return col;
			if (colVline[col] != 0)
				seeVline++;
			col++;
		}
	}
	else {
		for (; col < maxLen; col++) {
			if (colNchrs[col] && (colVline[col] ==	0))
				return col;
		}
	}
	return maxLen;
}
findFieldEnd(col)
int	col;
{
	for (; col < maxLen; col++) {
		if (wakuExist && colVline[col])
			return col;
		if (!wakuExist && colNchrs[col] == 0)
			return col;
	}
	return maxLen;
}
/*
 * Build a default format character for each field
 */
buildDefFormat(tblp)
register struct	table	*tblp;
{
	int	fld, col, delta;
	int	charsInVert;
	int	maxDeltaL, maxDeltaR;
	int	maxPosL,   maxPosR;
	for (fld = 1; fld < tbl_nfield; fld++) {
		charsInVert = 0;
		maxDeltaL = maxDeltaR = 0;
		for (col = tbl_field[fld].left; col < tbl_field[fld].right; col++)
			if (colNchrs[col] > charsInVert)
				charsInVert = colNchrs[col];
		if (charsInVert == 0) {
			tbl_field[fld].defFormat = 'c';
			continue;
		}
		for (col = tbl_field[fld].left; col < tbl_field[fld].right; col++) {
			delta = colNchrs[col] - (fld ? colNchrs[col-1] : 0);
			if (delta > maxDeltaL) {
				maxDeltaL = delta;
				maxPosL   = col;
			}
			if (-delta > maxDeltaR) {
				maxDeltaR = -delta;
				maxPosR   = col;
			}
		}
		if (colNchrs[col - 1] > maxDeltaR) {
			maxDeltaR = colNchrs[col - 1];
		}
		if ((maxDeltaL * 3 < charsInVert * 2)
		    &&	(maxDeltaR * 3 < charsInVert * 2))
			tbl_field[fld].defFormat = 'c';
		else if (maxDeltaL > maxDeltaR) {
			tbl_field[fld].defFormat = 'l';
			tbl_field[fld].align = maxPosL;
		}
		else if (maxDeltaL < maxDeltaR) {
			tbl_field[fld].defFormat = 'r';
			tbl_field[fld].align = maxPosR;
		}
		else if (maxDeltaL == maxDeltaR) {
			tbl_field[fld].defFormat = 'c';
		}
	}
}
/*
 * Build field
 */
buildField(begin, end, tblp)
int	begin;
int	end;
register struct	table	*tblp;
{
	register int	col;
	int	fld, l, nbody;
	int	lineExist;
	char	*s, *atr;

	maxLen = maxLength(begin, end);
	wakuAll	  = 1;
	wakuExist = 0;
	nbody	  = 0;
	lineHoriFull = lineHoriPart = 0;
	for (col = 0; col < maxLen; col++)
		colVline[col] = colNchrs[col] = 0;
	for (l = begin; l < end; l++) {
		int	blankStartCol = -1;
		int	lastIsChar = 0;
		if (texts[l]->blank)
			continue;
		switch (lineAttr(texts[l]->body, &atr)) {
		    case HORI_NULL:
			nbody++;
			lineExist = 0;
			break;
		    case HORI_DBL_ALL:
		    case HORI_SNGL_ALL:
			lineHoriFull++;
			lineExist = 1;
			break;
		    case HORI_EXIST:
			lineHoriPart++;
			lineExist = 1;
			break;
		}
		for (col = 0, s = texts[l]->body; *s; s++, atr++, col++) {
			if (*atr & ATR_JIS2) {
				colVline[col] |= COLV_JIS;
			}
			if (*atr & ATR_VLINE) {
				lineExist = 1;
				wakuExist++;
				colVline[col] |= COLV_ASC;
			}
			if (*atr & ATR_CHAR) {
				colNchrs[col]++;
				if (wakuExist
				    && blankStartCol != -1 && lastIsChar) {
					while (blankStartCol < col) {
						colNchrs[blankStartCol++]++;
					}
				}
				lastIsChar = 1;
			}
			else {
				if (lastIsChar && *atr)
					lastIsChar = 0;
			}
			if (*atr == 0) {
				blankStartCol = col;
			}
			else {
				blankStartCol = -1;
			}
		}
		if (!lineExist)
			wakuAll = 0;
	}
	tbl_field[0].defFormat = '-';
	tbl_field[0].right     = 0;
	tbl_field[0].left      = -1;
	tbl_nfield = 1;
	tbl_field[tbl_nfield].left  = findFieldLeft(0);
	while (tbl_field[tbl_nfield].left < maxLen) {
		tbl_field[tbl_nfield].right
			= findFieldEnd(tbl_field[tbl_nfield].left + 1);
		tbl_field[tbl_nfield + 1].left
			= findFieldLeft(tbl_field[tbl_nfield].right);
		tbl_nfield++;
		if (tbl_nfield >= TBL_MAX_FIELDS)
			return -1;	/* too many fields	*/
	}
	/* Find vertical lines	*/
	for (fld = 0; fld < tbl_nfield; fld++) {
		tbl_field[fld].vlines = 0;
		for (col = tbl_field[fld].right;
		     col < tbl_field[fld + 1].left; col++)
			if (colVline[col] == COLV_ASC)
				tbl_field[fld].vlines++;
	}
	expectedFields	= nbody * (tbl_nfield - 1);
	return 0;	/* OK	*/
}
fieldOnlyHline(lnum, fld, tblp)
int	lnum;
int	fld;
register struct	table	*tblp;
{
	int	i;
	char	*s;
	s = texts[lnum]->body + tbl_field[fld].left;
	for (i = tbl_field[fld].left; i < tbl_field[fld].right; i++, s++)
		if (*s != '-' && *s != '=' && *s != '+' && *s != '|')
			return 0;
	return 1;
}
shrinkField(begin, end, tblp)
int	begin;
int	end;
register struct	table	*tblp;
{
	int	i, l, fld;
	int	hattr;
	char	*atr;
	if (!wakuExist)
		return 0;
	for (i = 0; i < maxLen; i++)
		colPchrs[i] = 0;
	for (l = begin; l < end; l++) {
		if ((hattr = lineAttr(texts[l]->body, &atr))
		    == HORI_DBL_ALL || hattr == HORI_SNGL_ALL)
			continue;
		for (fld = 1; fld < tbl_nfield; fld++) {
			if (fieldOnlyHline(l, fld, tblp))
				continue;
			for (i = tbl_field[fld].left;
			     i < tbl_field[fld].right; i++)
				if (atr[i] != 0)
					colPchrs[i]++;
		}
	}
	for (fld = 1; fld < tbl_nfield; fld++) {
		while (tbl_field[fld].left < tbl_field[fld].right) {
			if (colPchrs[tbl_field[fld].left])
				break;
			else
				tbl_field[fld].left++;
		}
		while (tbl_field[fld].left < tbl_field[fld].right) {
			if (colPchrs[tbl_field[fld].right - 1])
				break;
			else
				tbl_field[fld].right--;
		}
	}
}
spaceInField(l, fld, tblp)
int	l;
int	fld;
struct	table	*tblp;
{
	int	j;
	int	stat = 0;
	for (j = tbl_field[fld].left;
	     j < tbl_field[fld].right && j < texts[l]->length; j++) {
		if (stat == 0 && *(texts[l]->body + j) != ' ')
			stat = 1;
		if (stat == 1 && *(texts[l]->body + j) == ' ')
			stat = 2;
		if (stat == 2 && *(texts[l]->body + j) != ' ')
			return 1;
	}
	if (stat == 0)
		return 1;
	return 0;
}
/*
 * Check if the specified region looks like table
 */
tblCheck(begin, end, tblp)
int	begin;
int	end;
register struct	table	*tblp;
{
	int	l, col, fld;
	int	nilg   = 0;
	int	spaces = 0;
	int	chars  = 0;
	int	lines  = 0;
	int	factor;
	factor = tableFactor * tableFactor / 50;
	if (!wakuAll) {
		if (tbl_nfield <= TBL_MIN_FIELDS)
			return 0;
		if (tbl_nfield * 100 > 12 * factor)
			return 0;
		for (l = begin; l < end; l++) {
			if (!texts[l]->blank) {
				lines  += texts[l]->picLines;
				spaces += texts[l]->spaces;
				chars  += texts[l]->length - texts[l]->indent;
			}
			for (fld = 0; fld < tbl_nfield; fld++)
				if (spaceInField(l, fld, tblp))
					nilg++;
		}
		DBG3(4, "tblCheck (%d-%d) ilg:%d\n", begin, end, nilg);

		if (((nilg * 25) > ((end - begin) * factor))
		    || (spaces * factor < chars * (japaneseText?15:10))
		    || ((lines * 25)> ((end - begin) * factor)))
			return 0;
	}
	else {
		char	*s;
#ifdef	undef
		if (tbl_nfield * 100 > 16 * factor)
			return 0;
#endif
		for (fld = 0; fld < tbl_nfield; fld++)
			if (tbl_field[fld].vlines > 2)
				return 0;
		if (tbl_field[0].vlines == 0 ||
		    tbl_field[tbl_nfield - 1].vlines == 0)
			return 0;
		if (lineHoriFull == 0)
			return 0;
		for (l = begin; l < end; l++) {
			s = texts[l]->body;
			for (col = 0; col < texts[l]->length; col++, s++) {
				if (colVline[col] == COLV_ASC
				    && tblAsciAttr[(unsigned char)*s] == 0
				    && tblKanjiByte[(unsigned char)*s]
				    && tblKwakuAttr(s) == 0)
					nilg++;
#ifdef	KANJI
				if (isZenkaku(s)) {
					s++;
					col++;
				}
#endif
			}
		}
		DBG3(4, "tblCheck (%d-%d) ilg:%d\n", begin, end, nilg);
	}
	if (nilg * 200 > expectedFields * factor)
		return 0;
	return 1;
}
markIfTable(begin, end)
int	begin;
int	end;
{
	struct	textBlock *tbp;
	struct	table	table;
	int	l;
	DBG2(2, "markIfTable (%d-%d)\n", begin, end);
	if (end - begin <= TBL_MIN_LINES)
		return 0;
	for (l = begin; l < end; l++)
		if (texts[l]->block)
			return 0;
	if (buildField(begin, end, &table))
		return 0;
	if (tblCheck(begin, end, &table) == 0)
		return 0;
	tbp = newTextBlock(begin, end, TB_TABLE);
	MSG2("%d-%d ", begin, end - 1);
	for (l = begin; l < end; l++)
		texts[l]->block = tbp;
	return 1;
}
extraColumn(atr, len, fld, tblp)
char	*atr;
int	len;
int	fld;
register struct	table	*tblp;
{
	int	fldLeft;
	int	i;
	fldLeft = fld;
	for (; fld < tbl_nfield; fld++) {
		if (wakuAll) {
			for (i = tbl_field[fld].right;
			     i < len && i < tbl_field[fld + 1].left; i++)
				if ((atr[i] & ATR_VLINE) != 0
				    || (atr[i] & ATR_HLINE) != 0)
					break;
			if (i < tbl_field[fld + 1].left)
				break;
		}
		else {
			if (tbl_field[fld].right >= len
			    || (atr[tbl_field[fld].right] & ATR_CHAR) == 0)
				break;
		}
	}
	if (fld == tbl_nfield)
		return tbl_nfield - fldLeft -1;
	else
		return fld - fldLeft;
}
formatOf(atr, len, fld, exCol, tblp)
char	*atr;
int	len;
int	fld;
int	exCol;
register struct	table	*tblp;
{
	if (tbl_field[fld].defFormat == 'l') {
		if (tbl_field[fld].left >= len
		    || atr[tbl_field[fld].left])
			return 'l';
		if (tbl_field[fld].left <= len) {
			if (tbl_field[fld + exCol].left
			    == tbl_field[fld + exCol].align) {
				if (atr[tbl_field[fld].left])
					return 'l';
			}
			else if ((atr[tbl_field[fld].align - 1] & ATR_CHAR)
				 == 0)
					return 'l';

		}
		if (tbl_field[fld + exCol].right <= len
		    && atr[tbl_field[fld + exCol].right - 1])
			return 'r';
	}
	else if (tbl_field[fld].defFormat == 'r') {
		if (tbl_field[fld + exCol].right <= len) {
			if (tbl_field[fld + exCol].right
			    == tbl_field[fld + exCol].align) {
				if(atr[tbl_field[fld + exCol].right - 1])
					return 'r';
			}
			else if ((atr[tbl_field[fld + exCol].right] &ATR_CHAR)
				 == 0){
				return 'r';
			}
		}
		if (tbl_field[fld].left >= len
		    || atr[tbl_field[fld].left])
			return 'l';
	}
	return 'c';
}
fieldEmpty(atr, len, fld, exCol, tblp)
char	*atr;
int	len;
int	fld;
int	exCol;
register struct	table	*tblp;
{
	int	i;
	for (i = tbl_field[fld].left; i < tbl_field[fld + exCol].right
	     && i < len; i++)
		if (atr[i] & ATR_CHAR)
			return 0;
	return 1;
}
char	*
tblSubstr(l, fld, exCol, tblp)
int	l;
int	fld;
int	exCol;
register struct	table	*tblp;
{
	static	char	buf[256];
	char	*to;
	char	*from;
	int	i;
	from = texts[l]->body + tbl_field[fld].left;
	to   = buf;
	for (i = tbl_field[fld].left; i < tbl_field[fld + exCol].right
	     && i < texts[l]->length; i++) {
		*to++ = *from++;
	}
	*to = '\0';
	return buf;
}
attrOfStr(atr, len, f, exCol, tblp)
char	*atr;
int	len;
int	f;
int	exCol;
register struct	table	*tblp;
{
	int	col;
	if (tbl_field[f + exCol].right >= len)
		return HORI_NULL;
	for (col = tbl_field[f].left; col < tbl_field[f + exCol].right; col++)
		if ((atr[col] & ATR_HLINE) == 0)
			return HORI_NULL;
	return HORI_SNGL_ALL;
}
tblOutput(begin, end)
int	begin;
int	end;
{
	int	ttop, tbtm;
	struct	table	table;

	PRINTED2(begin, end);
	/*
	 * Suppress Blank lines
	 */
	for (ttop = begin; ttop < end; ttop++) {
		if (!texts[ttop]->blank)
			break;
	}
	for (tbtm = end; tbtm > ttop; tbtm--) {
		if (!texts[tbtm - 1]->blank)
			break;
	}
	(void)buildField(ttop, tbtm, &table);/* Should Success */
	buildDefFormat(&table);
	shrinkField(ttop, tbtm, &table);
	(put->table)(ttop, tbtm, &table);
}

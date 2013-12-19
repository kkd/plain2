/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[]=
	"$Id: headfoot.c,v 2.9 1994/04/19 10:16:22 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include <ctype.h>
#include "plain2.h"
/*
 * Extract Page boundaries (including header & footer) from input text.
 */
#define	HF_DISTANCE	3	/* Max header/footer distance from ^L	*/
#define	HF_MAXLEN	64	/* Max length of header/footer string	*/
/*
 *			- xx -
 *  ^L
 *   plain2				1990/12/24
 */
struct	headerLine {
	char	left[HF_MAXLEN];
	char	right[HF_MAXLEN];
	char	center[HF_MAXLEN];
} origHeader, origFooter, curHeader, curFooter;
int	footerPos;
int	headerPos;
/*
 * Look for header/footer lines from page boundary (^L)
 */
pickUpLine(l, headerp, footerp, begin, end)
int	l;
char	**headerp;
char	**footerp;
int	begin;
int	end;
{
	int	d;
	for (d = 1; d < HF_DISTANCE && l - d > begin; d++)
		if (!texts[l - d]->blank) {
			*footerp = texts[l - d]->body;
			break;
		}
	footerPos = d;
	for (d = 1; d < HF_DISTANCE && l + d < end; d++)
		if (!texts[l + d]->blank) {
			*headerp = texts[l + d]->body;
			break;
		}
	headerPos = d;
}
/*
 * Two strings have same header/footer format
 *	(two lines are same except for page number field)
 */
sameFormat2(s1, s2)
char	*s1;
char	*s2;
{
	char	*t1, *t2;
	int	len1, len2;
	char	*s;
	DBG2(4, "sameFormat2 <%s><%s>", s1, s2);
	len1 = strlen(s1);
	len2 = strlen(s2);
	t1 = s1 + len1 - 1;
	t2 = s2 + len2 - 1;
	/*
	 * from left to right
	 */
	for (; *s1 && * s2; s1++, s2++)
		if (*s1 != *s2)
			break;
	/*
	 * from right to left
	 */
	for (; t1 >= s1 && t2 >= s2; t1--, t2--)
		if (*t1 != *t2)
			break;
	for (s = s1; s <= t1; s++)
		if (!isdigit(*s))
			return 0;
	for (s = s2; s <= t2; s++)
		if (!isdigit(*s))
			return 0;
	return 1;
}
sameFormat(hdr1, hdr2)
struct	headerLine	*hdr1;
struct	headerLine	*hdr2;
{
	return (sameFormat2(hdr1->left, hdr2->left)
		&& sameFormat2(hdr1->center, hdr2->center)
		&& sameFormat2(hdr1->right, hdr2->right));
}
/*
 * Build a Header field
 */
headerFields(str, hdrp)
char	*str;
struct	headerLine	*hdrp;
{
	char	*s, *t, *save;
	char	*centerLeft, *centerRight;
	/*
	 * from Center to Both side
	 */
	for (centerLeft = str + rightMargin * 2 / 5;
	     centerLeft >= str; centerLeft--) {
		if (centerLeft - 3 >= str
		    && !strncmp(centerLeft - 3, "   ", 3))
			break;
	}
	for (; centerLeft < str + strlen(str) ; centerLeft++)
		if (*centerLeft != ' ')
			break;
	if (centerLeft <= str || centerLeft > str + rightMargin * 3 / 5) {
		centerLeft = str + rightMargin / 5;
		centerRight = str + rightMargin / 2;
		*hdrp->center = '\0';
	}
	else {
		for (centerRight = centerLeft;
		     centerRight < str + strlen(str) ; centerRight++) {
			if (!strncmp(centerRight, "   ", 3)
			    || *centerRight == '\0')
				break;
		}
		for (save = hdrp->center, t = centerLeft;
		     t < centerRight; t++, save++)
			*save = *t;
		*save = '\0';
	}
	/*
	 * from Left to Center
	 */
	for (s = str; *s && s < centerLeft; s++) {
		if (!strncmp(s, "   ", 3))
			break;
	}
	for (save = hdrp->left, t = str; t < s; t++, save++)
		*save = *t;
	*save = '\0';
	/*
	 * from Right to Center
	 */
	for (s = str + strlen(str) - 1; s > centerRight; s--) {
		if (s - 3 >= str && !strncmp(s - 3, "   ", 3))
			break;
	}
	for (save = hdrp->right, t = s; t > centerRight
	     && t < str + strlen(str); t++, save++)
		*save = *t;
	*save = '\0';
}
build(str1, str2, fstr1, fstr2)
char	*str1;
char	*str2;
char	*fstr1;
char	*fstr2;
{
	char	*t1, *t2;
	char	*s1, *s2;
	int	len1, len2;
	char	*s;

	len1 = strlen(str1);
	len2 = strlen(str2);
	t1 = str1 + len1 - 1;
	t2 = str2 + len2 - 1;

	for (s1 = str1, s2 = str2, s = fstr1; *s1 && * s2; s1++, s2++) {
		if (*s1 != *s2)
			break;
		*s++ = *s1;
	}
	*s = '\0';
	if (*s1 == '\0' && *s2 == '\0') {
		/* str1 */
		return 0;
	}
	for (; s > fstr1; s--)
		if (!isdigit(*(s - 1)))
			break;
	*s = '\0';
	for (; t1 >= s1 && t2 >= s2; t1--, t2--) {
		if (*t1 != *t2)
			break;
	}
	for(; *t1; t1++)
		if (!isdigit(*t1))
			break;
	for (s = fstr2; *t1; t1++)
		*s++ = *t1;
	*s = '\0';
	/*
	 * left [page str] right
	 */
	return 1;
}
headerFooter(begin, end)
int	begin;
int	end;
{
	struct textBlock	*pbp;
	char	*headerStr, *footerStr;
	char	*header, *footer;

	if (pageBp == NULL)
		return;
	pickUpLine(pageBp->rbegin, &headerStr, &footerStr, begin, end);
	DBG4(3, "%d:1st-Header <%s>\n%d:1-stFooter<%s>\n",
	     pbp->rbegin, headerStr,
	     pbp->rbegin, footerStr);

	if (headerStr == NULL && footerStr == NULL) {
		/* No Header/Footer	*/
		overwritePaging(begin, end, 0, 0);
		return;
	}

	if (headerStr)
		headerFields(headerStr, &origHeader);
	if (footerStr)
		headerFields(footerStr, &origFooter);

	for (pbp = pageBp->nextBlock; pbp; pbp = pbp->nextBlock) {
		pickUpLine(pbp->rbegin, &header, &footer, begin, end);
		DBG4(3, "%d:Header <%s>\n%d:Footer<%s>\n",
		    pbp->rbegin, header,
		    pbp->rbegin, footer);
		if (headerStr && header != NULL) {
			headerFields(header, &curHeader);
			if (!sameFormat(&origHeader, &curHeader)) {
				overwritePaging(begin, end, 0, 0);
				return;
			}
		}
		if (footerStr && footer != NULL){
			headerFields(footer, &curFooter);
			if (!sameFormat(&origFooter, &curFooter)) {
				overwritePaging(begin, end, 0, 0);
				return;
			}
		}
	}
	overwritePaging(begin, end, headerPos, footerPos);
}
/*
 * Register Paging (including header/footer) as a NULL text block.
 * NULL block  will not be analyzed and printed anymore.
 */
overwritePaging(begin, end, hpos, fpos)
int	begin;
int	end;
int	hpos;
int	fpos;
{
	struct textBlock	*pbp;
	int	l, l1, l2;
	fprintf (stderr, "Followin Header & Footer Lines ignored\n");
	for (pbp = pageBp; pbp; pbp = pbp->nextBlock) {
		for (l1 = pbp->rbegin - fpos - 1;
		     l1 >= begin && l1 >= pbp->rbegin - fpos - 4; l1--)
			if (!texts[l1]->blank)
				break;
		for (l2 = pbp->rbegin + hpos + 1;
		     l2 < end && l2 <pbp->rbegin + hpos + 4; l2++)
			if (!texts[l2]->blank)
				break;
		for (l = l1 + 1 ; l < l2; l++) {
			texts[l]->block = pbp;
			texts[l]->indent = rightMargin;
			if (!texts[l]->blank)
				fprintf (stderr, "%d:%s\n", l, texts[l]->body);
		}
		pbp->rbegin = l1 + 1;
		pbp->rend   = l2;
		pbp->type   = TB_COMMENT;
	}
}
/*
 * Build and print Header/Footer string for troff output
 * (Note: This function returns a pointer to a static buffer which will be
 *  re-written at next call)
 */
extern struct transTable	roffTransQ[];
char	*
roffHdrStr(pat1, pat2)
char	*pat1, *pat2;
{
	char	left[HF_MAXLEN];
	char	right[HF_MAXLEN];
	static	char	ret[64];
	if (build(pat1, pat2, left, right)) {
		(void)strcpy(ret, codeCvt(textQuote(left, roffTransQ)));
		(void)strcat(ret, "\\\\\\\\nP");
		(void)strcat(ret, codeCvt(textQuote(right, roffTransQ)));
		return ret;
	}
	else
		return codeCvt(textQuote(left, roffTransQ));
}
roffPutHeader()
{
	if (headerPos) {
		printf(".PH \"'");
		printf ("%s'",   roffHdrStr(origHeader.left,  curHeader.left));
		printf ("%s'", roffHdrStr(origHeader.center,curHeader.center));
		printf ("%s'\"\n",roffHdrStr(origHeader.right, curHeader.right));
	}
	if (footerPos) {
		printf(".PF \"'");
		printf ("%s'", roffHdrStr(origFooter.left,  curFooter.left));
		printf ("%s'", roffHdrStr(origFooter.center,curFooter.center));
		printf ("%s'\"\n", roffHdrStr(origFooter.right, curFooter.right));
	}
}

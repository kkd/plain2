/*
 * Copyright (C) NEC Corporation 1991,1992
 */
#ifndef lint
static char rcsid[] =
	"$Id: analyze.c,v 2.13 1994/04/19 10:16:17 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include <ctype.h>
#include "plain2.h"
#include "picture.h"
#include "kanji.h"

/*
 * �ƹԤ��ʲ���ʸ��¤�Ȥ��Ƥ���ħ���Ĥ��ݤ���Ƚ�ꤹ��
 *	1) �ꥹ��
 *	2) ���ֹ�
 *	3) ��/ɽ�ֹ�
 */
struct strVal appendixPat[] = {
	/*
	 * ��Ͽ/Appendix
	 */
	{"��Ͽ", 0},
	{"Appendix", 0},
	{"APPENDIX", 0},
	{"����������", 0},
	{"���УУţΣģɣ�", 0},
	{"Appendix", 0},
	{"APPENDIX", 0},
	{"����������", 0},
	{"���УУţΣģɣ�", 0},
	{"", -1}
};
struct strVal zenkakuAlpUp[] = {
	{"��", 0},	{"��", 1},	{"��", 2},	{"��", 3},
	{"��", 4},	{"��", 5},	{"��", 6},	{"��", 7},
	{"��", 8},	{"��", 9},	{"��", 10},	{"��", 11},
	{"��", 12},	{"��", 13},	{"��", 14},	{"��", 15},
	{"��", 16},	{"��", 17},	{"��", 18},	{"��", 19},
	{"��", 20},	{"��", 21},	{"��", 22},	{"��", 23},
	{"��", 24},	{"��", 25},	{ "",  -1}};
struct strVal zenkakuAlpLow[] = {
	{"��", 0},	{"��", 1},	{"��", 2},	{"��", 3},
	{"��", 4},	{"��", 5},	{"��", 6},	{"��", 7},
	{"��", 8},	{"��", 9},	{"��", 10},	{"��", 11},
	{"��", 12},	{"��", 13},	{"��", 14},	{"��", 15},
	{"��", 16},	{"��", 17},	{"��", 18},	{"��", 19},
	{"��", 20},	{"��", 21},	{"��", 22},	{"��", 23},
	{"��", 24},	{"��", 25},	{ "",  -1}};
struct strVal bullet[] = {
	/*
	 * �ӥ��åȥꥹ�Ȥ˻Ȥ��ޡ���
	 */
	{ "��", 0},	{ "��", 0},
	{ "o ", 0},	{ "* ", 0},
	{ "+ ", 0},
	{ "",	-1}
};
struct strVal dash[] = {
	/*
	 * ���å���ꥹ�Ȥ˻Ȥ��ޡ���
	 */
	{ "��", 0},
	{ "- ", 0},
	{ "-",	0},
	{ "",	-1}
};
struct strVal listSpecial[] = {
	/*
	 * �ꥹ�Ȥ˻Ȥ��ü�ʥޡ��������ҥꥹ�ȤȤ��ư���
	 */
{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},	{ "��", 0},
	{ "��", 0},
	{ "",	-1}
};
/*
 * ���ѿ���
 */
struct strVal zenkakuNum[] = {
	{ "��", 0},	{ "��", 1},
	{ "��", 2},	{ "��", 3},
	{ "��", 4},	{ "��", 5},
	{ "��", 6},	{ "��", 7},
	{ "��", 8},	{ "��",	9},
	{ "",  -1}
};
/*
 * ���޿���
 */
struct strVal lroman[] = {
	{"��",	       10 },	{"�ɣ�",	9 },
	{"�֣ɣɣ�",	8 },	{"�֣ɣ�",	7 },
	{"�֣�",	6 },	{"��",		5 },
	{"�ɣ�",	4 },	{"�ɣɣ�",	3 },
	{"�ɣ�",	2 },	{"��",		1 },
	{"X",  10 },	{"IX",	9 },
	{"VIII",8 },	{"VII",	7 },
	{"VI",	6 },	{"V",	5 },
	{"IV",	4 },	{"III",	3 },
	{"II",	2 },	{"I",	1 },
	{ "",  -1}
};
struct strVal sroman[] = {
	{"��",	       10 },	{"���",	9 },
	{"������",	8 },	{"�����",	7 },
	{"����",	6 },	{"��",		5 },
	{"���",	4 },	{"����",	3 },
	{"���",	2 },	{"��",		1 },
	{"x",  10 },	{"ix",	9 },
	{"viii",8 },	{"vii",	7 },
	{"vi",	6 },	{"v",	5 },
	{"iv",	4 },	{"iii",	3 },
	{"ii",	2 },	{"i",	1 },
	{ "",  -1}
};
/*
 * �����
 */
struct strVal lparenpat[] = {
	{"��",	0},
	{"��",	0},
	{"(",	0},
	{"[",	0},
	{ "",  -1}
};
/*
 * �����(�ݤ��ä�)
 */
struct strVal rparenpat[] = {
	{"��",	0},
	{")",	0},
	{ "",  -1}
};
/*
 * �����(�֥�å��å�)
 */
struct strVal rbracketpat[] = {
	{"��",	0},
	{"]",	0},
	{ "",  -1}
};
/*
 * ���ѤΥɥå�/��
 */
struct strVal kdot[] = {
	{"��",	0},
	{"��",	0},
	{ "",  -1}
};

/*
 * ʸ������ͤ��Ȥߤι�¤�������ʸ����ΰ���
 * �ޥå�����ʸ�����ޤ๽¤�Τ��֤�
 */
struct	strVal	*
kstrMatch(str, kpat)
register char	*str;
register struct strVal	*kpat;
{
	register char	*patstr;
	register char	*s;
	for (; *kpat->pattern; kpat++) {
		patstr = kpat->pattern;
		s      = str;
		while (*patstr && *s) {
			if (*patstr++ != *s++)
				goto nextPat;
		}
		DBG2(9,"<%s> matched with <%s>\n", str, kpat->pattern);
		return	kpat;
	    nextPat:
		;
	}
	return NULL;
}
/*
 * Ⱦ�Ѥޤ������Ѥο���
 * �ޥå�����ʸ�����Ĺ��(1 or 2)���ޤ����԰���(0)���֤���
 */
checkIfNumber(str, rval)
char	*str;
int	*rval;
{
	struct	strVal	*svp;
	if (*str == '\0')
		return 0;
	if (index("0123456789", *str)) {
		*rval = *str - '0';
		return 1;
	}
	if (svp = kstrMatch(str, zenkakuNum)) {
		*rval = svp->value;
		return 2;
	}
	return 0;
}
/*
 * Ⱦ�Ѥޤ������ѤΥ��޿���
 * �ޥå�����ʸ�����Ĺ�����ޤ����԰���(0)���֤���
 */
checkIfRoman(str, rval, typep)
register char	*str;
int	*rval;
int	*typep;
{
	struct	strVal	*svp;
	if (svp = kstrMatch(str, lroman))
		*typep = L_LROMAN;
	else if (svp = kstrMatch(str, sroman))
		*typep = L_SROMAN;
	else
		return 0;
	*rval = svp->value;
	return strlen(svp->pattern);
}
checkIfAppendix(str, nump)
char	*str;
int	*nump;
{
	struct strVal	*svp;
	int	len, len2;
	int	ttt;
#define	skipSpaces()	while (*str == ' ') {str++;len++;}
	if ((svp = kstrMatch(str, appendixPat)) == NULL)
		return 0;
	len = strlen(svp->pattern);
	str += len;
	skipSpaces();
	while (svp = kstrMatch(str, appendixPat)) {
		len2 = strlen(svp->pattern);
		len += len2;
		str += len2;
	}
	skipSpaces();
	if ((len2 = checkIfRoman(str, nump, &ttt))
	    || (len2 = checkIfNumStr(str, nump))
	    || (len2 = checkIfAlpha(str, nump, &ttt))
	    || (len2 = checkIfZenkakuAlpha(str, nump, &ttt))) {
		len += len2;
		str += len2;
	}
	else
		return 0;
	skipSpaces();
	if (svp = kstrMatch(str, kdot))
		len += strlen(svp->pattern);
	else if (*str == '.')
		len++;
	skipSpaces();
	return len;
}
/*
 * ����������ֹ�Ȥ��Ƥ���ħ�������Ƥ��뤫
 *
 *     ����(����/Ⱦ��) + �ɥå�(����/Ⱦ��)	�� ������(�ꥹ�Ȥ��⤷��ʤ�)
 *     ���� + �ɥå� + ����			�� OK
 *     ���� + �ɥå� + ���� + �ɥå�		�� OK
 *     ���� + �ɥå� + ���� + �ɥå� + ��	�� OK
 *
 */
checkIfSecNumber(str, dots)
register char	*str;
int	*dots;
{
	struct	strVal	*svp;
	int	ofst, tofst, len;
	int	intermDots;
	int	dummy;
	intermDots = 0;
	ofst = 0;
	while (1) {
		tofst = 0;
		while (len = checkIfNumber(str+ofst+tofst, &dummy))
			tofst += len;
		ofst += tofst;
		if (tofst == 0) {
			*dots = intermDots;
			return ofst;
		}
		if ((len = (index(".", *(str + ofst)) != NULL))
		    || (svp = kstrMatch(str+ofst, kdot))
		    && (len = strlen(svp->pattern))) {
			ofst += len;
			intermDots++;
		}
		else {
			if (intermDots && index(" ", *(str+ofst))) {
				*dots = intermDots + 1;
				return ofst + 1;
			}
#ifdef	KANJI
			else if (isZenkaku(str)
				 && index(" ", *(str + ofst))) {
				*dots = intermDots + 1;
				return ofst + 1;
			}
#endif
			else
				return 0;
		}
	}
}
/*
 * �������ݤ�
 * ���� lnump �ˤϿ������͡��꥿�����ͤˤ�
 * �ޥå�����������Ĺ�����֤�
 */
checkIfNumStr(str, lnump)
char	*str;
int	*lnump;
{
	int	ofst, len;
	int	num;
	*lnump = 0;
	ofst = 0;
	while (len = checkIfNumber(str + ofst, &num)){
		*lnump = *lnump * 10 + num;
		ofst += len;
	}
	return ofst;
}
/*
 * Ⱦ�ѥ���ե��٥åȤ��ݤ�
 * ���� lnump �ˤϡ�����ե��٥åȤΰ���(a=1, b=2, ��)���֤�
 * typep����ʸ��/��ʸ���ζ��̤��֤�
 * �ޥå�����ʸ����Ĺ��(0 or 1)���֤�
 */
checkIfZenkakuAlpha(str, lnump, typep)
char	*str;
int	*lnump;
int	*typep;
{
	struct strVal	*svp;
	if (svp = kstrMatch(str, zenkakuAlpUp)) {
		*lnump = svp->value;
		*typep = L_LALPHA;
		return 2;
	}
	else if (svp = kstrMatch(str, zenkakuAlpLow)) {
		*lnump = svp->value;
		*typep = L_SALPHA;
		return 2;
	}
	return 0;
}
/*
 * Ⱦ�ѥ���ե��٥åȤ��ݤ�
 * ���� lnump �ˤϡ�����ե��٥åȤΰ���(a=1, b=2, ��)���֤�
 * typep����ʸ��/��ʸ���ζ��̤��֤�
 * �ޥå�����ʸ����Ĺ��(0 or 1)���֤�
 */
checkIfAlpha(str, lnump, typep)
char	*str;
int	*lnump;
int	*typep;
{
	if (isupper(*str)) {
		*lnump = alphaVal(tolower(*str));
		*typep = L_LALPHA;
		return 1;
	}
	else if (islower(*str)) {
		*lnump = alphaVal(*str);
		*typep = L_SALPHA;
		return 1;
	}
	return 0;
}
/*
 * ��󷿥ꥹ�ȤȤ��Ƥ���ħ�������Ƥ��뤫
 *
 *     {���������޿�����Ⱦ�ѥ���ե��٥å�} + �ɥå�
 *     {���������޿�����Ⱦ�ѥ���ե��٥å�} + �����
 *     ����� + {���������޿�����Ⱦ�ѥ���ե��٥å�}  + �����
 *
 * �ꥹ���ֹ���ʬ��Ĺ�����֤�
 */
checkIfEnumList(str, textp)
register char	*str;
register struct	text	*textp;
{
	register int	len;
	struct strVal	*svp;
	int	ofst, lparen, num, type;
	int	alphabetic = 0;
	lparen = 0;
	if (svp = kstrMatch(str, lparenpat)) {
		lparen = strlen(svp->pattern);
	}
	/* Enumeration body	*/
	if (len = checkIfRoman(str + lparen, &num, &type)) {
		textp->listType = type;
		textp->listNum	= num;
	}
	else if (len = checkIfNumStr(str + lparen, &num)) {
		textp->listType = L_NUMBER;
		textp->listNum	= num;
	}
	else if (len = checkIfAlpha(str + lparen, &num, &type)) {
		textp->listType = type;
		textp->listNum	= num;
		alphabetic = 1;
	}
	if (len == 0)
		return 0;
	ofst = lparen + len;
	if (lparen == 0 && (len = (index(".", *(str + ofst))) != NULL)) {
		if (alphabetic && !japaneseText)
			/* Special rule for non-japanese text	*/
			return 0;
		textp->listHint = LH_DOTTED;
	}
	else if (svp = kstrMatch(str+ofst, rparenpat)) {
		len = strlen(svp->pattern);
		if (lparen)
			textp->listHint = LH_PAREN;
		else
			textp->listHint = LH_RPAREN;
	}
	else if (svp = kstrMatch(str+ofst, rbracketpat)) {
		len = strlen(svp->pattern);
		if (lparen)
			textp->listHint = LH_BRACKET;
		else
			textp->listHint = LH_RBRACKET;
	}
	else return 0;
	ofst += len;
	if (textp->listType == L_NUMBER && checkIfNumStr(str + ofst, &num))
		return 0;
	return ofst;
}
/*
 * ���ҷ��ꥹ�ȤȤ��Ƥ���ħ�������Ƥ��뤫
 *
 *     "[" + �����ȥ� + "]" + ����		<== OK
 *     "[" + �����ȥ� + "]" + ���� + ʸ��	<== OK
 *     "[" + �����ȥ� + "]" + ʸ��		<== ����
 *     �����ȥ� + ":" + ����		<== OK
 *     �����ȥ� + ":" + ���� + ʸ��	<== OK
 *     �����ȥ� + ":" + ʸ��		<== ����
 *
 *  �㳰Ū������
 *     "[" + �����ȥ� + ":" +  "]"	<=== "[" ���� "]"�ޤǤ򥿥��ȥ����Ȥ���
 *     "[" + �����ȥ� "]" + ���� + ":"	<=== "[" ���� "]"�ޤǤ򥿥��ȥ����Ȥ���
 */
checkIfDscrList(str)
register char	*str;
{
	register char	*markp;
	int	colonLen = 0, bracketLen = 0;
	markp = index(str, ':');
	if (markp && (index(markp, ' ') == markp + 1))
		colonLen = markp - str + 1;
	else if (markp && *(markp+1) == '\0')
		colonLen =  markp - str + 1;
	if (*str == '[') {
		markp = index(str, ']');
		if (markp && (index(markp, ' ') == markp + 1))
			bracketLen = markp - str + 1;
		else if (markp && *(markp+1) == '\0')
			bracketLen = markp - str + 1;
	}
	if (bracketLen)
		return bracketLen;
	else
		return colonLen;
}
/*
 * �ꥹ�Ȥޤ��ϥ���������ֹ�Ȥ��Ƥ���ħ�������Ƥ��뤫
 * ���������Ƥ���С��Ԥ�°���Ȥ�����Ͽ���롥
 *	���ꥹ�Ȥȥ���������ֹ�ξ�郎������������Ʊ����Ĵ�٤Ƥ���
 *	�㤨�� "1. xxx"�Ȥ����Ԥϡ��ɤ���ξ���������
 */
lineAtrListSec(textp)
register struct	text	*textp;
{
	register char	*str;
	struct strVal	*svp;
	int	len;
	int	dots;
	str = textp->body + textp->indent;
	if (svp = kstrMatch(str, bullet)) {
		len = strlen(svp->pattern);
		if (*(str + len)) {
			DBG1(7,"Matched with BULLET %s\n", str);
			textp->pListHead = DEFINITELY;
			textp->listType	 = L_BULLET;
			textp->headLen	 = len;
		}
	}
	else if (svp = kstrMatch(str, dash)) {
		len = strlen(svp->pattern);
		if (kstrMatch(str + len, dash) == 0
		    && *(str + len)
		    && (len != 1 || textp->japanese)) {
			/* "-" AMBIGUOUS list head. but "------" is not	*/
			DBG1(7,"Matched with DASH %s\n", str);
			if (len == 1)
				textp->pListHead = AMBIGUOUS;
			else
				textp->pListHead = DEFINITELY;
			textp->listType	 = L_DASH;
			textp->headLen	 = len;
		}
	}
	else if (len = checkIfEnumList(str, textp)) {
		DBG1(7,"Matched with ENUM_LIST=%s\n", str);
		textp->pListHead = AMBIGUOUS;
		textp->headLen	 = len;
	}
	if (indentedSecnum || (textp->indent == 0)) {
		if (len = checkIfSecNumber(str, &dots)) {
			DBG3(7,"Matched with SEC_NUM(%d,%d) %s\n",
			     len, dots, str);
			if (dots > 1
#ifdef	KANJI
			    || maybeZenkakuNum(textp->body)
#endif
			    ){
				/* x.x or (Zenkaku Number...)	*/
				textp->pSecNum = DEFINITELY;
			}
			else {
				textp->pSecNum = AMBIGUOUS;
			}
			textp->headLen	= len;
			textp->secDepth = dots;
		}
	}
}
/*
 * ���ҷ��ꥹ�ȤȤ��Ƥ���ħ�������Ƥ���С��Ԥ�°���Ȥ�����Ͽ���롥
 */
lineAtrDlist(textp)
register struct	text	*textp;
{
	int	ret;
	if (textp->pListHead)
		return;
	if (ret = checkIfDscrList(textp->body + textp->indent)) {
		DBG1(7,"Matched with DSCR_LIST %s\n", textp->body);
		if (ret < MAX_DSCRLEN) {
			textp->pListHead = DEFINITELY;
			textp->listType	 = L_DLIST;
			textp->headLen	 = ret;
		}
	}
	{
		struct strVal	*svp;
		if (svp = kstrMatch(textp->body + textp->indent, listSpecial)){
			char	*s;
			ret = strlen(svp->pattern);
			s = textp->body + textp->indent + ret;
		
			if (*s
#ifdef	PICTURE
			    && picLineMatch(s) == NULL
			    && picMiscMatch(s) == NULL
#endif
			    ) {
				DBG1(7,"Matched with DSCR_LIST(special) %s\n",
				     textp->body);
				textp->pListHead = DEFINITELY;
				textp->listType	 = L_DLIST;
				textp->headLen	 = ret;
			}
		}
	}
}
/*
 * ʸ���󸡺����®�����뤿�ᡤ�����оݤȤʤ�ʸ��������Х��Ȥ�
 * ��Ͽ���Ƥ���
 */
byteRegister(reg, kpat)
register unsigned char	reg[];
struct	strVal	*kpat;
{
	for (; *kpat->pattern; kpat++) {
		reg[(unsigned char)*kpat->pattern]++;
	}
}
/*
 * �ƹԤ���ħ�򤢤餫���ḡ�Ф��Ƥ���
 */
analyzeLines(begin, end)
int	begin;
int	end;
{
	int	i, l;
	unsigned char	firstByteListSec[256];
	short	lengthAccum[MAX_LINE_LEN];
	register struct	text	*textp;
	DBG0(5,"checkLineCharacter\n");
	bzero((char *)lengthAccum, sizeof(lengthAccum));
	/*
	 * �ꥹ�ȡ��ޤ��ϥ���������ֹ�Ȥ��Ƥθ���򤷤ܤ뤿��
	 * ���Х��Ȥ���Ͽ���Ƥ���
	 */
	bzero((char *)firstByteListSec, sizeof(firstByteListSec));
	byteRegister(firstByteListSec, bullet);
	byteRegister(firstByteListSec, dash);
	byteRegister(firstByteListSec, zenkakuNum);
	byteRegister(firstByteListSec, sroman);
	byteRegister(firstByteListSec, lroman);
	byteRegister(firstByteListSec, lparenpat);
	for (i = '0'; i <= '9'; i++)
		firstByteListSec[i]++;
	for (i = 'a'; i <= 'z'; i++)
		firstByteListSec[i]++;
	for (i = 'A'; i <= 'Z'; i++)
		firstByteListSec[i]++;
	for (l = begin; l < end; l++) {
		textp = texts[l];
		if (textp->blank)
			continue;
		lengthAccum[textp->length]++;
		if (firstByteListSec[(unsigned char)*(textp->body + textp->indent)]) {
			lineAtrListSec(textp);
		}
		if (textp->pSecNum == NEVER)
			lineAtrDlist(textp);
		lineAtrFtitle(textp);
#ifdef	PICTURE
		picCharCount(textp);
#endif
	}
	/*
	 * �饤�ȥޡ�����򸡽Ф��Ƥ���
	 * ���Τ�90%�ιԤ����ޤ���֤�饤�ȥޡ�����Ȥ��Ƥ���
	 */
	for (i = 1; i < MAX_LINE_LEN; i++)
		lengthAccum[i] += lengthAccum[i - 1];
	for (i = 0; i < MAX_LINE_LEN; i++)
		if ((long)lengthAccum[i] * 100
		    > (long)lengthAccum[MAX_LINE_LEN - 1] * 90){
			rightMargin = i;
			break;
		}
	MSG1("Right Margin = %d\n", rightMargin);
}
	

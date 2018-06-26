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
 * 各行が以下の文書構造としての特徴をもつか否かを判定する
 *	1) リスト
 *	2) 章番号
 *	3) 図/表番号
 */
struct strVal appendixPat[] = {
	/*
	 * 付録/Appendix
	 */
	{"付録", 0},
	{"Appendix", 0},
	{"APPENDIX", 0},
	{"Ａｐｐｅｎｄｉｘ", 0},
	{"ＡＰＰＥＮＤＩＸ", 0},
	{"Appendix", 0},
	{"APPENDIX", 0},
	{"Ａｐｐｅｎｄｉｘ", 0},
	{"ＡＰＰＥＮＤＩＸ", 0},
	{"", -1}
};
struct strVal zenkakuAlpUp[] = {
	{"Ａ", 0},	{"Ｂ", 1},	{"Ｃ", 2},	{"Ｄ", 3},
	{"Ｅ", 4},	{"Ｆ", 5},	{"Ｇ", 6},	{"Ｈ", 7},
	{"Ｉ", 8},	{"Ｊ", 9},	{"Ｋ", 10},	{"Ｌ", 11},
	{"Ｍ", 12},	{"Ｎ", 13},	{"Ｏ", 14},	{"Ｐ", 15},
	{"Ｑ", 16},	{"Ｒ", 17},	{"Ｓ", 18},	{"Ｔ", 19},
	{"Ｕ", 20},	{"Ｖ", 21},	{"Ｗ", 22},	{"Ｘ", 23},
	{"Ｙ", 24},	{"Ｚ", 25},	{ "",  -1}};
struct strVal zenkakuAlpLow[] = {
	{"ａ", 0},	{"ｂ", 1},	{"ｃ", 2},	{"ｄ", 3},
	{"ｅ", 4},	{"ｆ", 5},	{"ｇ", 6},	{"ｈ", 7},
	{"ｉ", 8},	{"ｊ", 9},	{"ｋ", 10},	{"ｌ", 11},
	{"ｍ", 12},	{"ｎ", 13},	{"ｏ", 14},	{"ｐ", 15},
	{"ｑ", 16},	{"ｒ", 17},	{"ｓ", 18},	{"ｔ", 19},
	{"ｕ", 20},	{"ｖ", 21},	{"ｗ", 22},	{"ｘ", 23},
	{"ｙ", 24},	{"ｚ", 25},	{ "",  -1}};
struct strVal bullet[] = {
	/*
	 * ビュレットリストに使うマーク
	 */
	{ "・", 0},	{ "＊", 0},
	{ "o ", 0},	{ "* ", 0},
	{ "+ ", 0},
	{ "",	-1}
};
struct strVal dash[] = {
	/*
	 * ダッシュリストに使うマーク
	 */
	{ "−", 0},
	{ "- ", 0},
	{ "-",	0},
	{ "",	-1}
};
struct strVal listSpecial[] = {
	/*
	 * リストに使う特殊なマーク．記述リストとして扱う
	 */
{ "→", 0},	{ "←", 0},
	{ "☆", 0},	{ "★", 0},
	{ "▽", 0},	{ "▼", 0},
	{ "△", 0},	{ "▲", 0},
	{ "□", 0},	{ "■", 0},
	{ "◇", 0},	{ "◆", 0},
	{ "●", 0},	{ "◯", 0},
	{ "○", 0},	{ "◎", 0},
	{ "※", 0},
	{ "",	-1}
};
/*
 * 全角数字
 */
struct strVal zenkakuNum[] = {
	{ "０", 0},	{ "１", 1},
	{ "２", 2},	{ "３", 3},
	{ "４", 4},	{ "５", 5},
	{ "６", 6},	{ "７", 7},
	{ "８", 8},	{ "９",	9},
	{ "",  -1}
};
/*
 * ローマ数字
 */
struct strVal lroman[] = {
	{"Ｘ",	       10 },	{"ＩＸ",	9 },
	{"ＶＩＩＩ",	8 },	{"ＶＩＩ",	7 },
	{"ＶＩ",	6 },	{"Ｖ",		5 },
	{"ＩＶ",	4 },	{"ＩＩＩ",	3 },
	{"ＩＩ",	2 },	{"Ｉ",		1 },
	{"X",  10 },	{"IX",	9 },
	{"VIII",8 },	{"VII",	7 },
	{"VI",	6 },	{"V",	5 },
	{"IV",	4 },	{"III",	3 },
	{"II",	2 },	{"I",	1 },
	{ "",  -1}
};
struct strVal sroman[] = {
	{"ｘ",	       10 },	{"ｉｘ",	9 },
	{"ｖｉｉｉ",	8 },	{"ｖｉｉ",	7 },
	{"ｖｉ",	6 },	{"ｖ",		5 },
	{"ｉｖ",	4 },	{"ｉｉｉ",	3 },
	{"ｉｉ",	2 },	{"ｉ",		1 },
	{"x",  10 },	{"ix",	9 },
	{"viii",8 },	{"vii",	7 },
	{"vi",	6 },	{"v",	5 },
	{"iv",	4 },	{"iii",	3 },
	{"ii",	2 },	{"i",	1 },
	{ "",  -1}
};
/*
 * 左括弧
 */
struct strVal lparenpat[] = {
	{"（",	0},
	{"［",	0},
	{"(",	0},
	{"[",	0},
	{ "",  -1}
};
/*
 * 右括弧(丸かっこ)
 */
struct strVal rparenpat[] = {
	{"）",	0},
	{")",	0},
	{ "",  -1}
};
/*
 * 右括弧(ブラッケット)
 */
struct strVal rbracketpat[] = {
	{"］",	0},
	{"]",	0},
	{ "",  -1}
};
/*
 * 全角のドット/丸
 */
struct strVal kdot[] = {
	{"．",	0},
	{"。",	0},
	{ "",  -1}
};

/*
 * 文字列と値の組みの構造体配列と文字列の一致
 * マッチした文字列を含む構造体を返す
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
 * 半角または全角の数字
 * マッチした文字列の長さ(1 or 2)，または不一致(0)を返す．
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
 * 半角または全角のローマ数字
 * マッチした文字列の長さ，または不一致(0)を返す．
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
 * セクション番号としての特徴を備えているか
 *
 *     数字(全角/半角) + ドット(全角/半角)	← 怪しい(リストかもしれない)
 *     数字 + ドット + 数字			← OK
 *     数字 + ドット + 数字 + ドット		← OK
 *     数字 + ドット + 数字 + ドット + …	← OK
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
 * 数字か否か
 * 引数 lnump には数字の値，リターン値には
 * マッチした数字の長さを返す
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
 * 半角アルファベットか否か
 * 引数 lnump には，アルファベットの位置(a=1, b=2, …)を返す
 * typepに大文字/小文字の区別を返す
 * マッチした文字の長さ(0 or 1)を返す
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
 * 半角アルファベットか否か
 * 引数 lnump には，アルファベットの位置(a=1, b=2, …)を返す
 * typepに大文字/小文字の区別を返す
 * マッチした文字の長さ(0 or 1)を返す
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
 * 列挙型リストとしての特徴を備えているか
 *
 *     {数字，ローマ数字，半角アルファベット} + ドット
 *     {数字，ローマ数字，半角アルファベット} + 右括弧
 *     左括弧 + {数字，ローマ数字，半角アルファベット}  + 右括弧
 *
 * リスト番号部分の長さを返す
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
 * 記述型リストとしての特徴を備えているか
 *
 *     "[" + タイトル + "]" + 改行		<== OK
 *     "[" + タイトル + "]" + 空白 + 文章	<== OK
 *     "[" + タイトル + "]" + 文章		<== だめ
 *     タイトル + ":" + 改行		<== OK
 *     タイトル + ":" + 空白 + 文章	<== OK
 *     タイトル + ":" + 文章		<== だめ
 *
 *  例外的ケース
 *     "[" + タイトル + ":" +  "]"	<=== "[" から "]"までをタイトル部とする
 *     "[" + タイトル "]" + 何か + ":"	<=== "[" から "]"までをタイトル部とする
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
 * リストまたはセクション番号としての特徴を備えているか
 * 条件を備えていれば，行の属性として登録する．
 *	列挙リストとセクション番号の条件がかちあうため同時に調べている
 *	例えば "1. xxx"という行は，どちらの条件も満たす
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
 * 記述型リストとしての特徴を備えていれば，行の属性として登録する．
 */
lineAtrDlist(textp)
register struct	text	*textp;
{
	int	ret;
	if (textp->pListHead)
		return 0;
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
 * 文字列検査を高速化するため，検査対象となる文字列の第一バイトを
 * 登録しておく
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
 * 各行の特徴をあらかじめ検出しておく
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
	 * リスト，またはセクション番号としての候補をしぼるため
	 * 第一バイトを登録しておく
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
	 * ライトマージンを検出しておく
	 * 全体の90%の行が収まる位置をライトマージンとしている
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
	

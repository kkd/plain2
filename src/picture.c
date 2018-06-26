/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
/*
 * modify by k-chinen@is.aist-nara.ac.jp, 1994
 */

#ifndef lint


#endif

#ifdef	PICTURE
#include <stdio.h>
#include "plain2.h"
#include "kanji.h"
#include "picture.h"
#include <ctype.h>
/*
 * JISの罫線素片で描かれた線画を，troff用にはPIC, LaTeX用に picture
 * に変換して出力する
 */
#define	IS_FULL_LINE(x)	((x)==LINE_FULL|| (x)==LINE_SEP ||\
			 (x)==VECT_BGN || (x)==VECT_END)
struct	picLineElm {
	char	*pattern;
	struct	stroke	xline;
	struct	stroke	yline;
} picLineElm[] = {
#ifdef	undef
	{"⇔", {LINE_FULL,SY_CIRCLE},	{LINE_NULL,SY_NULL}},
#endif
	{"￣", {LINE_FULL,SY_ELPS},	{LINE_NULL,SY_NULL}},
	{"＿", {LINE_FULL,SY_HELPS},	{LINE_NULL,SY_NULL}},
	{"←", {VECT_BGN, SY_NULL},	{LINE_NULL,SY_NULL}},
	{"→", {VECT_END, SY_NULL},	{LINE_NULL,SY_NULL}},
	{"↑", {LINE_NULL,SY_NULL},	{VECT_BGN, SY_NULL}},
	{"↓", {LINE_NULL,SY_NULL},	{VECT_END, SY_NULL}},
	{"：", {LINE_NULL,SY_NULL},	{LINE_FULL,SY_DASH}},
	{"…", {LINE_FULL,SY_DASH},	{LINE_NULL,SY_NULL}},
	{"‖", {LINE_NULL,SY_NULL},	{LINE_FULL,SY_DBL }},
	{"＝", {LINE_FULL,SY_DBL},	{LINE_NULL,SY_NULL}},
	{"─", {LINE_FULL,SY_THIN},	{LINE_NULL,SY_NULL}},
	{"━", {LINE_FULL,SY_THICK},	{LINE_NULL,SY_NULL}},
	{"│", {LINE_NULL,SY_NULL},	{LINE_FULL,SY_THIN}},
	{"┃", {LINE_NULL,SY_NULL},	{LINE_FULL,SY_THICK}},
	{"┌", {LINE_BGN, SY_NULL},	{LINE_BGN, SY_NULL}},
	{"┏", {LINE_BGN, SY_THICK},	{LINE_BGN, SY_THICK}},
	{"┐", {LINE_END, SY_NULL},	{LINE_BGN, SY_NULL}},
	{"┓", {LINE_END, SY_THICK},	{LINE_BGN, SY_THICK}},
	{"└", {LINE_BGN, SY_NULL},	{LINE_END, SY_NULL}},
	{"┗", {LINE_BGN, SY_THICK},	{LINE_END, SY_THICK}},
	{"┘", {LINE_END, SY_NULL},	{LINE_END, SY_NULL}},
	{"┛", {LINE_END, SY_THICK},	{LINE_END, SY_THICK}},
	{"├", {LINE_BGN, SY_NULL},	{LINE_SEP, SY_NULL}},
	{"┝", {LINE_BGN, SY_THICK},	{LINE_SEP, SY_NULL}},
	{"┠", {LINE_BGN, SY_NULL},	{LINE_FULL,SY_THICK}},
	{"┣", {LINE_BGN, SY_THICK},	{LINE_FULL,SY_THICK}},
	{"┤", {LINE_END, SY_NULL},	{LINE_SEP, SY_NULL}},
	{"┥", {LINE_END, SY_THICK},	{LINE_SEP, SY_NULL}},
	{"┨", {LINE_END, SY_NULL},	{LINE_FULL,SY_THICK}},
	{"┫", {LINE_END, SY_THICK},	{LINE_FULL,SY_THICK}},
	{"┬", {LINE_SEP, SY_NULL},	{LINE_BGN, SY_NULL}},
	{"┳", {LINE_FULL,SY_THICK},	{LINE_BGN, SY_THICK}},
	{"┯", {LINE_FULL,SY_THICK},	{LINE_BGN, SY_NULL}},
	{"┰", {LINE_SEP, SY_NULL},	{LINE_BGN, SY_THICK}},
	{"┴", {LINE_SEP, SY_NULL},	{LINE_END, SY_NULL}},
	{"┻", {LINE_FULL,SY_THICK},	{LINE_END, SY_THICK}},
	{"┷", {LINE_FULL,SY_THICK},	{LINE_END, SY_NULL}},
	{"┸", {LINE_SEP, SY_NULL},	{LINE_END, SY_THICK}},
	{"┼", {LINE_SEP, SY_NULL},	{LINE_SEP, SY_NULL}},
	{"┿", {LINE_FULL,SY_THICK},	{LINE_SEP, SY_NULL}},
	{"╂", {LINE_SEP, SY_NULL},	{LINE_FULL,SY_THICK}},
	{"╋", {LINE_FULL,SY_THICK},	{LINE_FULL,SY_THICK}},
	{""  ,	0, 0, 0, 0}};
struct	picMiscLines {
	char	*pattern;
	short	x0, y0;
	short	x1, y1;
} picMiscLines[] = {
	{"／", 0,2, 2,0},	{"＼", 0,0, 2,2},
	{"￣", 0,0, 2,0},	{"＿", 0,2, 2,2},
	{"×", 0,0, 0,0},
	{""  , 0,0, 0,0}};
static	int	vpos;
static	int	colPos;

int	picFontSize;
enum	horiVert {hv_horizon, hv_vertical} horiVert;

static unsigned char firstBytePicLine[256];
static unsigned char firstBytePicMisc[256];
picParseInit()
{
	register struct	picLineElm	*lelp;
	register struct	picMiscLines	*mlp;
	bzero((char *)firstBytePicLine, sizeof(firstBytePicLine));
	bzero((char *)firstBytePicMisc, sizeof(firstBytePicMisc));
	for (lelp = picLineElm; *lelp->pattern; lelp++)
		firstBytePicLine[(unsigned char)*lelp->pattern] = 1;
	for (mlp = picMiscLines; *mlp->pattern; mlp++)
		firstBytePicMisc[(unsigned char)*mlp->pattern] = 1;
}
picOutText(s, length)
char	*s;
int	length;
{
	static char	buf[256];
	int	width;
	int	fieldWidth;
	(void)strncpy(buf, s, length);
	buf[length] = '\0';
	width = strWidth(buf);
	
	fieldWidth = strlen(buf);
	for (s += length; *s == ' '; s++)
	     fieldWidth++;
	if (*s == '\0')
		fieldWidth = width;	/* 右側はあいている	*/
	(put->picText)(buf, colPos - length, vpos, width / fieldWidth);
}
/*
 * 出力イメージでの文字列の幅を
 * おおよその値で求める
 */
strWidth(s)
char	*s;
{
	int	width = 0;
	for (; *s; s++) {
		if (isZenkaku(s)) {
			s++;
			width += 10;		/* Kanji */
		}
		else if (islower(*s))
			width += 10;		/* Lower case		*/
		else if (isupper(*s))
			width += 14;		/* Zenkaku alphabet	*/
		else if (index("!\"'(),-./:;[]{|}^`", *s))
			width += 7;		/* Special characters	*/
		else
			width += 11;		/* Others "[+=#@...]"	*/
	}
	return width;
}

struct picLineElm	*
picLineMatch(s)
register char	*s;
{
	register struct	picLineElm	*lelp;
	if (firstBytePicLine[(unsigned char)*s] == 0)
		return NULL;
	lelp = picLineElm;
	while (*lelp->pattern) {
		if (str2match(s, lelp->pattern))
			return lelp;
		lelp++;
	}
	return NULL;
}
struct picMiscLines *
picMiscMatch(s)
register char	*s;
{
	register struct	picMiscLines	*mlp;
	if (firstBytePicMisc[(unsigned char)*s] == 0)
		return NULL;
	mlp = picMiscLines;
	while (*mlp->pattern) {
		if (str2match(s, mlp->pattern))
			return mlp;
		mlp++;
	}
	return NULL;
}
/*
 * 線画に使う文字が一行に何文字含まれているかを数える
 * 罫線素片文字と，一部の特殊文字
 */
picCharCount(textp)
register struct	text	*textp;
{
	register char	*str;
	/*
	 * 罫線素片およびいくつかの特殊文字が一行にいくつ含まれているか
	 */
	if  (textp->japanese) {
		str = textp->body + textp->indent;
		while (*str) {
			if (firstBytePicLine[(unsigned char)*str] ||
			    firstBytePicMisc[(unsigned char)*str]) {
				if (picMiscMatch(str) || picLineMatch(str))
					textp->picLines++;
				str++;
			}
			else if (isZenkaku(str))
				str++;
			str++;
		}
		DBG2(4, "picCharCount %2d:%s\n",
		     textp->picLines, textp->body + textp->indent);
	}
}
/*
 * ばらばらの罫線素片による直線を，一本の線で表すための処理
 */
static int	xorg, yorg, linePend, lineStyle, vectAtr;
newLine(stp, half)
struct	stroke	*stp;
int	half;
{
	xorg = colPos;
	yorg = vpos;
	if (horiVert == hv_horizon)
		xorg += half;
	else
		yorg += half;
	linePend  = 1;
	lineStyle = stp->style;
	vectAtr	  = (stp->type == VECT_BGN);
}
terminateLine(stp)
struct	stroke	*stp;
{
	int	xend, yend;
	if (!linePend)
		return 0;
	linePend = 0;
	if (vectAtr && (stp && stp->type == VECT_END))
		vectAtr = VECT_BOTH;
	else if (stp && stp->type == VECT_END)
		vectAtr = VECT_END;
	xend = colPos;
	yend = vpos;
	if (horiVert == hv_horizon) {
		yorg++;
		yend++;
		if (stp) {
			xend++;
			if (stp->type != LINE_END && stp->type != LINE_SEP)
				xend++;
		}
	}
	else if (horiVert == hv_vertical) {
		xorg++;
		xend++;
		if (stp) {
			yend++;
			if (stp->type != LINE_END && stp->type != LINE_SEP)
				yend++;
		}
	}
	(put->picLine)(xorg, yorg, xend, yend, lineStyle, vectAtr);
}
processLine(stp)
struct	stroke	*stp;
{
	switch(stp->type) {
	    case VECT_END:
	    case LINE_END:
	    case LINE_SEP:
		if (!linePend )
			newLine(stp, 0);
		if (lineStyle && stp->style && lineStyle != stp->style) {
			terminateLine(NULL);
			newLine(stp, 1);
		}
		terminateLine(stp);
		if (stp->type == LINE_SEP) {
			terminateLine(NULL);
			newLine(stp, 1);
		}
		break;
	    case LINE_BGN:
		terminateLine(NULL);
		newLine(stp, 1);
		break;
	    case VECT_BGN:
		terminateLine(NULL);
		newLine(stp, 0);
		break;
	    case LINE_FULL:
		if (!linePend ) {
			newLine(stp, 0);
			break;
		}
		if (lineStyle && stp->style && lineStyle != stp->style){
			terminateLine(NULL);
			newLine(stp, 0);
		}
		else
			lineStyle = stp->style;
		break;
	    case LINE_NULL:
		terminateLine(NULL);
		break;
	    default:
		printf("????\n");
		break;
	}
}
char	*
subcIfKanji(col, lnum)
int	col;
int	lnum;
{
	if (col < 0 || lnum < 1 || lnum >= textLines)
		return	"";
	if (isKanjiChar(texts[lnum], col))
		return texts[lnum]->body + col;
	else
		return "";
}
picMiscLinePut(mlp)
struct	picMiscLines	*mlp;
{
	(put->picLine)(colPos + mlp->x0,vpos + mlp->y0,
		       colPos + mlp->x1,vpos + mlp->y1,
		       SY_THIN, 0);
}
/*
 * 斜め線を直線と接続するための特殊処理
 */
slantCnctOut(xdir, ydir, turnX, turnY, lnum)
int	xdir, ydir;
int	turnX, turnY;
int	lnum;
{
	struct	picLineElm	*lelp;
	struct	stroke	*stp;
	if ((lelp = picLineMatch(subcIfKanji(colPos + turnX * 2, lnum + turnY)))
	    == NULL)
		return 0;
	if (turnX == 0)
		stp = &lelp->yline;
	else
		stp = &lelp->xline;
	if (IS_FULL_LINE(stp->type)) {
		(put->picLine)(colPos + 1, vpos + 1,
			       colPos + 1 - xdir * 2, vpos + 1 - ydir * 2,
			       SY_THIN, 0);
		(put->picLine)(colPos + 1, vpos + 1,
			       colPos + 1 + turnX * 2, vpos + 1 + turnY * 2,
			       stp->style, 0);
		return 1;
	}
	return 0;
}
/*
 *  斜め線を伸ばして表示する
 */
slantExtent(xdir, ydir, lnum)
int	xdir, ydir;
int	lnum;
{
	struct	picLineElm	*lelp;
	int	hit = 0;
	if ((lelp = picLineMatch(subcIfKanji(colPos + xdir * 2, lnum + ydir)))
	    != NULL) {
		hit++;
		if (!IS_FULL_LINE(lelp->yline.type)
		    && !IS_FULL_LINE(lelp->xline.type))
			return 0;
	}
	if ((lelp = picLineMatch(subcIfKanji(colPos + xdir * 1, lnum + ydir)))
	    != NULL) {
		hit++;
		if (!IS_FULL_LINE(lelp->xline.type))
			return 0;
	}
	if (hit)
		(put->picLine)
			(colPos + 1, vpos + 1,
			 /* 本当は
			  * "colPos + 1 + xdir, vpos + 1 + ydir"
			  * だが，LaTeX 出力ではこんな短い線は書けないので
			  */
			 colPos + 1 + xdir * 2, vpos + 1 + ydir * 2,
			 SY_THIN, 0);
}
slantConnect(xdir, ydir, l, mlp)
int	xdir;
int	ydir;
int	l;
struct	picMiscLines	*mlp;
{
	int	leftTurnX, leftTurnY;
	int	rightTurnX, rightTurnY;
	leftTurnX = ( xdir + ydir) / 2;
	leftTurnY = (-xdir + ydir) / 2;
	rightTurnX = (xdir - ydir) / 2;
	rightTurnY = (xdir + ydir) / 2;
	
	if (slantCnctOut(xdir, ydir, leftTurnX, leftTurnY, l) != 0
	    || slantCnctOut(xdir, ydir, rightTurnX, rightTurnY, l) != 0)
		return 0;
	slantExtent(xdir, ydir, l);
	picMiscLinePut(mlp);
}
slantArrow(lnum)
int	lnum;
{
	int	xdir, ydir;
	int	hit = 0;
	for (xdir = -1; xdir <=1; xdir += 2)
		for (ydir = -1; ydir <=1; ydir += 2) {
			if (!strncmp(subcIfKanji(colPos + xdir * 2,
						 lnum + ydir ),
				     (xdir * ydir > 0)?"＼":"／", 2)) {
				hit++;
				(put->picLine)(colPos + 1 - xdir,
					       vpos   + 1 - ydir,
					       colPos + 1 + xdir,
					       vpos   + 1 + ydir,
					       SY_THIN, VECT_BGN);
			}
		}
	return hit;
}
picMisc(mlp, lnum)
int	lnum;
struct	picMiscLines	*mlp;
{
	struct	picLineElm	*lelp;
	int	lup, ldown, lleft, lright;
	int	sup, sdown, sleft, sright;
	char	*s;
	lup = ldown = lleft = lright = 0;
	sup = sdown = sleft = sright = 0;
	if (str2match(mlp->pattern, "×")) {
		if (slantArrow(lnum))
			return 1;
		else
			return 0;
	}
	else if (str2match(mlp->pattern, "／")) {
		if (!strncmp(subcIfKanji(colPos + 2, lnum - 1), "／", 2)) {
			s = subcIfKanji(colPos - 2, lnum + 1);
			if (str2match(s, "／") || str2match(s, "×")){
				/*	／／／	  */
				picMiscLinePut(mlp);
				return 1;
			}
			else {
				/*	□／／	 */
				slantConnect( -1, 1, lnum, mlp);
				return 1;
			}
		}
		else if (!strncmp(subcIfKanji(colPos - 2, lnum + 1), "／", 2)){
			/*	／／□		*/
			slantConnect(1, -1, lnum, mlp);
			return 1;
		}
	}
	else if (str2match(mlp->pattern, "＼")) {
		if (!strncmp(subcIfKanji(colPos - 2, lnum - 1), "＼", 2)) {
			s = subcIfKanji(colPos + 2, lnum + 1);
			if (str2match(s, "＼") || str2match(s, "×")){
				/*	＼＼＼	*/
				picMiscLinePut(mlp);
				return 1;
			}
			else {
				/*	＼＼□	*/
				slantConnect(1, 1, lnum, mlp);
				return 1;
			}
		}
		else if (!strncmp(subcIfKanji(colPos + 2, lnum + 1), "＼", 2)) {
			/*	□＼＼	*/
			slantConnect(-1, -1, lnum, mlp);
			return 1;
		}
	}
	if (lelp = picLineMatch(subcIfKanji(colPos , lnum - 1))) {
		lup = IS_FULL_LINE(lelp->yline.type);
		sup = lelp->yline.style;
	}
	if (lelp = picLineMatch(subcIfKanji(colPos , lnum + 1))) {
		ldown = IS_FULL_LINE(lelp->yline.type);
		sdown = lelp->yline.style;
	}
	if (lelp = picLineMatch(subcIfKanji(colPos + 2 , lnum))) {
		lright = IS_FULL_LINE(lelp->xline.type);
		sright = lelp->xline.style;
	}
	if (lelp = picLineMatch(subcIfKanji(colPos - 2, lnum))) {
		lleft = IS_FULL_LINE(lelp->xline.type);
		sleft = lelp->xline.style;
	}
#define	STYLE_MERGE(s1, s2)	((s1 == s2)?s1:SY_THIN)
	if (str2match(mlp->pattern, "／")) {
		int	hit = 0;
		if (lup && lleft) {
			hit++;
			(*put->picArc)(colPos, vpos, 1, dir_br,
				       STYLE_MERGE(sup, sleft));
		}
		if (ldown && lright) {
			hit++;
			(*put->picArc)(colPos + 2 , vpos + 2, 1,dir_tl,
				       STYLE_MERGE(sdown, sright));
		}
		if (hit)
			return 1;

	}
	if (str2match(mlp->pattern, "＼")) {
		int	hit = 0;
		if (lup && lright) {
			hit++;
			(*put->picArc)(colPos + 2 , vpos, 1, dir_bl,
				       STYLE_MERGE(sup, sright));
		}
		if (ldown && lleft) {
			hit++;
			(*put->picArc)(colPos , vpos + 2, 1, dir_tr,
				       STYLE_MERGE(sdown, sleft));
		}
		if (hit)
			return 1;
	}
	picMiscLinePut(mlp);
	return 1;
}
parseHoriLine(lnum)
int	lnum;
{
	struct	picLineElm	*lelp;
	struct	picMiscLines	*mlp;
	register char	*textBegin = NULL;
	register char	*s;
	colPos	 = texts[lnum]->indent;
	linePend = 0;
	for (s = texts[lnum]->body + texts[lnum]->indent;
	     colPos < texts[lnum]->length; s++, colPos++) {
		if (lelp = picLineMatch(s)) {
			processLine(&lelp->xline);
			if (textBegin) {
				picOutText(textBegin, s - textBegin);
				textBegin = NULL;
			}
		}
		else if ((mlp = picMiscMatch(s)) && picMisc(mlp,lnum)){
			terminateLine(NULL);
			if (textBegin) {
				picOutText(textBegin, s - textBegin);
				textBegin = NULL;
			}
		}
		else {
			terminateLine(NULL);
			if (textBegin && *s == ' ') {
				picOutText(textBegin, s - textBegin);
				textBegin = NULL;
			}
			if (textBegin == NULL && *s != ' ') {
				textBegin = s;
			}
		}
		if (isZenkaku(s)) {
			s++;
			colPos++;
		}
	}
	terminateLine(NULL);
	if (textBegin)
		picOutText(textBegin, s - textBegin);
}
maybePicture(begin, end)
int	begin;
int	end;
{
	int	nwaku = 0, wakuLines = 0;
	int	spaceSum = 0, lengthSum = 0;
	int	l, indent;
	DBG2(2, "maybePicture (%d-%d)\n", begin, end);
	if ((end - begin < 3) &&
	    !((prevLine(begin)->block &&
	       prevLine(begin)->block->type == TB_PICTURE)))
	    return 0;

	indent = minIndent(begin, end);
	for (l = begin; l < end; l++) {
		spaceSum  += texts[l]->spaces;
		lengthSum += texts[l]->length - indent;
		if (texts[l]->block)
			return 0;
		if (texts[l]->picLines)
			wakuLines++;
		nwaku += texts[l]->picLines;
	}
	DBG3(4, "maybePicture (%d-%d) %d\n", begin, end, nwaku);

	/* 
	 * Sun4 C コンパイラのバグ回避
	 * ここを "return (.. && .. && ..)" のように書き直すと
	 * Sun4 C コンパイラ は誤ったコードを吐く
	 */
	if (lengthSum
	    && (wakuLines * 3 > (end - begin))
	    && (nwaku > (end - begin))
	    &&	(((nwaku * 100 / (end - begin))
		  + (spaceSum * 800) / lengthSum) > 150))
		return 1;
	else
		return 0;

}
markIfPicture(begin, end)
int	begin;
int	end;
{
	struct	textBlock *tbp;
	int	l;
	if (maybePicture(begin, end)) {
		tbp = newTextBlock(begin, end, TB_PICTURE);
		MSG2("%d-%d ", begin, end - 1);
		for (l = begin; l < end; l++)
			texts[l]->block = tbp;
		return 1;
	}
	return 0;
}
picOutput(begin, end)
int	begin;
int	end;
{
	int	l;
	int	maxLen;
	int	minInd;

#ifdef HTML
        static int pic_count=0;
        if(put == &htmlPut) {
	    if(htmlOnce) {
		if(rawOutput || !htmlOld) { /* KK */
		    printf("<PRE>\n");
		    for( l = begin; l < end; l++) {
			htmlRawText(texts[l]->body);
		    }
		    printf("</PRE>\n");
		}
		else {
		    printf("<!-- PICTURE -->\n");
		}
		return 0;
	    }
	    else {
		printf("<!-- plain2:PICTURE %05d %d %d -->\n",
		    pic_count, begin, end);
		if(htmlHere) {
		    printf("<P><IMG SRC=\"PIC%05d.gif\">\n", pic_count);
		}
		else {
	    printf("<P><B><A HREF=\"PIC%05d.gif\">Picture here</A></B></P>\n",
			    pic_count);
		}
		pic_count++;
		return 0;
	    }
        }
#endif /* HTML */

	minInd = minIndent(begin, end);
	maxLen = maxLength(begin, end);
	if ((maxLen - minInd) * fontSize > pageWidth)
		picFontSize = pageWidth / (maxLen - minInd);
	else
		picFontSize = fontSize;
	(put->pictureBlock)(BEGIN, end - begin, minInd, maxLen);
	/*
	 * 横方向の処理．
	 * 直線，テキスト，その他の線
	 */
	horiVert = hv_horizon;
	for (vpos = 0, l = begin ; l < end ; l++, vpos+=2) {
		parseHoriLine(l);
	}
	/*
	 * 縦方向の処理
	 */
	horiVert = hv_vertical;
	for (colPos = minInd; colPos < maxLen; colPos++) {
		struct	picLineElm	*lelp;
		linePend = 0;
		for (vpos = 0, l = begin; l < end; l++, vpos+=2) {
			if (texts[l]->length <= colPos - 1) {
				terminateLine(NULL);
				continue;
			}
			if (isKanjiChar(texts[l], colPos)
			    && (lelp = picLineMatch(texts[l]->body + colPos)))
				processLine(&lelp->yline);
			else
				terminateLine(NULL);
		}
		terminateLine(NULL);
	}
	(put->pictureBlock)(END, 0);
}
#endif

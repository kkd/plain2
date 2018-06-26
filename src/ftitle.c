/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[]=
	"$Id: ftitle.c,v 2.12 1994/04/19 10:16:21 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"

extern struct strVal kdot[];
extern struct	strVal	*kstrMatch();
/*
 * Figure/Table title strings
 */
#define	MARK_FIG	1
#define	MARK_TBL	2
struct strVal figTblMark[] = {
#ifdef	KANJI
	{"¿Þ",		FT_FIGURE },
	{"É½",		FT_TABLE },
#endif
	{"Figure",	FT_FIGURE },
	{"Fig.",	FT_FIGURE },
	{"fig. ",	FT_FIGURE },
	{"Table",	FT_TABLE },
	{ "",  -1}
};
unsigned char	firstByteFtitle[256];
ftitleInit()
{
	bzero((char *)firstByteFtitle, sizeof(firstByteFtitle));
	byteRegister(firstByteFtitle, figTblMark);
}
lineAtrFtitle(textp)
register struct	text	*textp;
{
	struct	strVal	*svp;
	int	ofst, len, len2, num, fnum;
	char	*str;

	if (textp->pListHead || textp->indent == 0)
		return 0;
	if (firstByteFtitle[(unsigned char)*(textp->body + textp->indent)]
	    == 0)
		return 0;
	str = textp->body + textp->indent;
	if (svp = kstrMatch(str, figTblMark)) {
		textp->fTitle = svp->value;
	}
	else return 0;

	len = strlen(svp->pattern);
	while (*(str + len) == ' ')
		len++;
	str  += len;
	if (*str == '\0')
		goto undo;
	ofst = 0;
	fnum = 0;
	while (len2 = checkIfNumber(str + ofst, &num)){
		fnum = fnum * 10 + num;
		ofst += len2;
	}
	if (ofst == 0)
		goto undo;
	textp->listNum = fnum;
	if (kstrMatch(str + ofst, kdot))
		ofst += 2;
	else if (*(str + ofst) == '.')
		ofst++;
	while (*(str + ofst) == ' ')
		ofst++;
	textp->headLen = ofst + len;
	return 0;
    undo:
	textp->fTitle = 0;
	return 0;
}
figTitle(begin, end)
int	begin;
int	end;
{
	int	i;
	for (i = begin; i < end; i++) {
		if (texts[i]->block == NULL
		    && texts[i]->fTitle) {
			texts[i]->block = newTextBlock(i, i + 1, TB_FTITLE);
		}
	}
}
/*
 * connect figure/picture block with its title
 */
titledBlock(tbp1, tbp2)
struct	textBlock	*tbp1;
struct	textBlock	*tbp2;
{
	struct	textBlock	*newTbp;

	newTbp = newTextBlock(tbp1->rbegin, tbp2->rend, TB_CAPSULE);
	texts[tbp1->rbegin]->block = newTbp;

	newTbp->nextBlock = tbp1;
	tbp1->nextBlock = tbp2;
	tbp2->nextBlock = NULL;

	tbp1->superBlock = tbp2->superBlock = newTbp;
}
joinIfTitled(titlep, begin, end)
struct	text		*titlep;
int	begin;
int	end;
{
	struct	textBlock	*tbpPrev   = NULL;
	struct	textBlock	*tbpFollow = NULL;
	struct	textBlock	*titleTbp  = titlep->block;
	DBG1(7, "joinIfTitled (%d)\n", titleTbp->rbegin); 

	tbpPrev   = prevBlock(titleTbp->rbegin, begin);
	tbpFollow = nextBlock(titleTbp->rend, end);
	
	if (tbpPrev && ((tbpPrev->type != TB_TABLE
			&& tbpPrev->type != TB_PICTURE)
			|| tbpPrev->superBlock))
		tbpPrev = NULL;
	if (tbpFollow && ((tbpFollow->type != TB_TABLE
			  && tbpFollow->type != TB_PICTURE)
			  || tbpFollow->superBlock))
		tbpFollow = NULL;

	DBG2(4, "joinIfTitled (p:%d,f:%d)\n", tbpPrev, tbpFollow);

	if ((tbpPrev && !tbpFollow)
	    || ((tbpPrev && tbpFollow) 
		&& ((titleTbp->rbegin - tbpPrev->rend) 
		    < (tbpFollow->rbegin - titleTbp->rbegin)))) {
		titledBlock(tbpPrev, titleTbp);
		return titleTbp->rend;
	}
	else if (tbpFollow) {
		titledBlock(titleTbp, tbpFollow);
		return tbpFollow->rend;
	}
	return titleTbp->rend;
}
makeTitledBlock(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end;) {
		if (texts[l]->block && texts[l]->block->type == TB_FTITLE) {
			l = joinIfTitled(texts[l], begin, end);
		}
		else
			l++;
	}
}

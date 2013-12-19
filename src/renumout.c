/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: renumout.c,v 2.11 1994/04/19 10:17:01 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"

/*
 * Renumber list and section numbers
 */
#define	bodyStr(textp)	((textp)->blank\
			 ?(textp)->body\
			 :(textp)->body+(textp)->indent)

#ifdef	KANJI
char	*list_sroman[] = {"£É", "£É£É", "£É£É£É", "£É£Ö",
				  "£Ö","£Ö£É","£Ö£É£É","£Ö£É£É£É","£É£Ø","£Ø"};
char	*list_lroman[] = {"£é", "£é£é", "£é£é£é£é", "£é£ö",
				  "£ö","£ö£é","£ö£é£é","£ö£é£é£é","i£ø","£ø"};
#else
char	*list_sroman[] = {"I", "II", "III", "IV",
				  "V","VI","VII","VIII","IX","X"};
char	*list_lroman[] = {"i", "ii", "iiii", "iv",
				  "v","vi","vii","viii","ix","x"};
#endif
char	*list_salph[] = {"a","b","c","d","e","f","g","h","i","j","k",
				 "l","m","n","o","p","q","r","s","t","u",
				 "v","w","x","y","z"};
char	*list_lalph[] = {"A","B","C","D","E","F","G","H","I","J","K",
				 "L","M","N","O","P","Q","R","S","T","U",
				 "V","W","X","Y","Z"};
#define	LMAX_SROMAN	(sizeof(list_sroman)/sizeof(char *))
#define	LMAX_LROMAN	(sizeof(list_lroman)/sizeof(char *))
#define	LMAX_SALPHA	(sizeof(list_salph)/sizeof(char *))
#define	LMAX_LALPHA	(sizeof(list_lalph)/sizeof(char *))

static	listLevel = 0;

renumPrint(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end; l++)
		printf ("%s\n", texts[l]->body);
}

char	*
listNumStr(lbp, num)
struct textBlock *lbp;
{
	static char	buf[32];
	char	*numstr, *listFormat;
	switch (texts[lbp->rbegin]->listType) {
	    case L_BULLET:
	    case L_DASH:
	    case L_DLIST:
		break;
	    case L_SROMAN:
		numstr = list_sroman[num> LMAX_SROMAN? LMAX_SROMAN:num];
		goto moreStyle;
	    case L_LROMAN:
		numstr = list_lroman[num> LMAX_LROMAN? LMAX_LROMAN:num];
		goto moreStyle;
	    case L_LALPHA:
		numstr = list_lalph[num> LMAX_LALPHA? LMAX_LALPHA:num];
		goto moreStyle;
	    case L_SALPHA:
		numstr = list_salph[num> LMAX_SALPHA? LMAX_SALPHA:num];
		goto moreStyle;
	    case L_NUMBER:
		{
			static char numb[8];
			sprintf(numb, "%d", num+1);
			numstr = numb;
			goto moreStyle;
		}
	    moreStyle:
		switch (texts[lbp->rbegin]->listHint) {
		    case LH_PAREN:
			listFormat = "(%s)";
			break;
		    case LH_RPAREN:
			listFormat = "%s)";
			break;
		    case LH_BRACKET:
			listFormat = "[%s]";
			break;
		    case LH_RBRACKET:
			listFormat = "%s]";
			break;
		    case LH_DOTTED:
			listFormat = "%s.";
			break;
		    default:
			listFormat = "%s";
			break;
		}
		break;
	}
	sprintf(buf, listFormat, numstr);
	return buf;
}
renumList(lbp)
struct textBlock *lbp;
{
	struct	textBlock *lhbp;
	int	lnum, rend;
	int	list_num = 0;
	listLevel++;

	for (lhbp = lbp->nextBlock; lhbp != NULL; lhbp = lhbp->nextBlock) {
		lnum = lhbp->rbegin;
		/* renum first block (list head block)	*/
		if (texts[lnum]->listType == L_DLIST
		    || texts[lnum]->listType == L_DASH
		    || texts[lnum]->listType == L_BULLET) {
			printf ("%s\n", texts[lnum]->body);
		}
		else {
			int i;
			for (i=0; i<texts[lnum]->indent; i++)
				putchar(' ');
			printf ("%s %s\n", listNumStr(lhbp, list_num++),
				listSecBody(texts[lnum]));
		}
		PRINTED2(lnum,  lhbp->rend);
		renumPrint(lnum + 1, lhbp->rend);
		/* renum rest of the list body	*/
		if (lhbp->nextBlock != NULL) {
			rend = lhbp->nextBlock->rbegin;
		}
		else
			rend = lbp->rend;
		renumRegion(lhbp->rend, rend);
	}
	listLevel--;
}
#define	SEC_MAX_DEPTH	8
static int	lastDepth = SEC_MAX_DEPTH;
static int	snum[SEC_MAX_DEPTH] = {0,0,0,0,0,0,0,0,};
initSnum(textp)
struct	text	*textp;
{
	char	*snumStr;
	int	i, num;
	snumStr = textp->body + textp->indent;
	for (i = 1; i <= textp->secDepth; i++) {
		snumStr = strNum(snumStr, &num);
		if (num != 1
		    || textp->secDepth != 1) {
			if (i == textp->secDepth)
				num--;
			snum[i - 1] = num;
		}
	}
}
renumSection(depth, body)
int	depth;
char	*body;
{
	int	i;
	if (depth >= SEC_MAX_DEPTH)
		depth = SEC_MAX_DEPTH - 1;

	for (i = lastDepth - 1; i >= depth; i--)
		snum[i] = 0;

	snum[depth - 1]++;
	for (i = 0; i < depth; i++)
		printf ("%d.", snum[i]);
	printf (" %s\n", body);
}
renumFtitle(ft, indent, str)
int	ft;
int	indent;
char	*str;
{
	static int	figNumber = 1;
	static int	tblNumber = 1;
	int	i;
	for (i = 0; i < indent; i++)
		putchar(' ');
#ifdef	KANJI
	if (japaneseText) {
		if (ft == FT_FIGURE)
			printf ("%s %d.", codeCvt("¿Þ"), figNumber++);
		else if (ft == FT_TABLE)
			printf ("%s %d.", codeCvt("É½"), tblNumber++);
	}
	else {
#endif
		if (ft == FT_FIGURE)
			printf ("Figure %d.}", figNumber++);
		else if (ft == FT_TABLE)
			printf ("Table %d.}", tblNumber++);
#ifdef	KANJI
	}
#endif
	printf (" %s\n", str);
}

renumCapsule(blockTbp)
struct	textBlock	*blockTbp;
{
	struct	textBlock	*tbp1, *tbp2;
	int	l;

	tbp1 = blockTbp->nextBlock;
	tbp2 = tbp1->nextBlock;
	renumBlock(tbp1);
	for (l = tbp1->rend; l < tbp2->rbegin; l++) {
		if (texts[l]->block == NULL && texts[l]->blank)
			PRINTED(l);
	}
	renumBlock(tbp2);
}
renumBlock(tbp)
struct	textBlock *tbp;
{
	int	l;
	l = tbp->rbegin;
	switch (tbp->type) {
	    case TB_SECNUM:
		renumSection(texts[l]->secDepth, listSecBody(texts[l]));
		PRINTED(l);
		break;
	    case TB_FTITLE:
		renumFtitle(texts[l]->fTitle,
			    texts[l]->indent,
			    texts[l]->body + texts[l]->indent
			    + texts[l]->headLen);
		PRINTED(l);
		break;
	    case TB_PAGE:
	    case TB_APPENDIX:
		printf ("%s\n", texts[l]->body);
		PRINTED(l);
		break;
	    case TB_COMMENT:
	    case TB_SPACE:
	    case TB_RIGHT:
	    case TB_CENTER:
	    case TB_QUOTE:
	    case TB_EXAMPLE:
	    case TB_PLAIN:
	    case TB_TABLE:
	    case TB_PICTURE:
	    case TB_RAW:
		renumPrint(l, tbp->rend);
		PRINTED2(l, tbp->rend);
		break;
	    case TB_LISTHD:
		printf("ERROR(List Renum%d)%s\n", l, texts[l]->body);
		break;
	    case TB_LIST:
		renumList(tbp);
		break;
	    case TB_CAPSULE:
		renumCapsule(tbp);
		break;
		break;
	    default:
		fprintf(stderr, "PANIC(unknown in renum)\n");
		exit(2);
	}
}
renumRegion(begin, end)
int	begin;
int	end;
{
	int	l;
	struct	textBlock	*tbp;
	l = begin;
	if (begin >= end)
		return;
	while (l < end) {
		tbp = texts[l]->block;
		if (tbp) {
			renumBlock(tbp);
			l = tbp->rend;
		}
		else {
			if (!texts[l]->blank)
				printf("undef%d:%s\n", l, texts[l]->body);
			else {
				printf ("\n");
				PRINTED(l);
			}
			l++;
		}
	}
}
renumDocument()
{
	int	l;
	for (l = 1; l < textBegin; l++)
		printf ("%s\n", texts[l]->body);

	for (l = textBegin; l < textLines; ) {
		if (texts[l]->block) {
			if (texts[l]->block->type == TB_SECNUM) {
				initSnum(texts[l]);
				break;
			}
			l = texts[l]->block->rend;
		}
		else
			l++;
	}
	renumRegion(textBegin, textLines);
}

/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: list.c,v 2.9 1994/04/19 10:16:47 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
/*
 * 1st step:
 *	Find list head lines
 * 2nd step:
 *	Fix list region (containing other text blocks)
 */
findListHead(begin, end)
int	begin;
int	end;
{
	int	indent, l, curType, curHint;
	struct	textBlock *listbp, *lhbp, *lastbp;
	int	blankPassed = 0;
	int	prevListNum;
	DBG2(7, "findListHead (%d-%d)\n", begin, end);
	curType = texts[begin]->listType;
	curHint = texts[begin]->listHint;
	indent  = texts[begin]->indent;
	/*
	 * LIST structure
	 *	LIST-+- -> LIST HEAD	"1) list1"
	 *	     				xxx
	 *	     	-> LIST HEAD    "2) list2"
	 *	     
	 *		-> NULL
	 */
	listbp = newTextBlock(begin, begin+1, TB_LIST);
	lhbp   = newTextBlock(begin, begin+1, TB_LISTHD);
	listbp->nextBlock = lhbp;
	lhbp->superBlock  = listbp;
	lastbp        = lhbp;
	prevListNum   = texts[begin]->listNum;
	for (l = begin + 1; l < end; l++) {
		int romanInAlpha;
		if (texts[l]->block == NULL && texts[l]->pListHead != NEVER) {
			/* special case for "i" and "I"	*/
			if (((curType == L_SALPHA
			      && texts[l]->listType == L_SROMAN)
			     || (curType == L_LALPHA
				 && texts[l]->listType == L_LROMAN))
			    && prevListNum == alphaVal('h')
			    && texts[l]->listNum  == 1)
				romanInAlpha = 1;
			else
				romanInAlpha = 0;
			/* end of special case		*/
			if ((texts[l]->listType == curType || romanInAlpha)
			    && texts[l]->listHint == curHint
			    && texts[l]->indent == indent) {
				if (curType <= L_ENUM_MAX
				    && (texts[l]->listNum != ++prevListNum)) {
					/* Number/Alpha List not continuous */
					if (!romanInAlpha
					    && texts[l]->listNum == 1)
						break;
					MSG1("\nWarning: non continuous list at line %d\n", l);
					MSG1("\t%s\n",codeCvt(texts[l]->body));
					if (romanInAlpha)
						prevListNum = alphaVal('i');
					else
						prevListNum =texts[l]->listNum;
				}
				/* Continuation of same list	*/
				lhbp = newTextBlock(l, l + 1 ,TB_LISTHD);
				lastbp->nextBlock = lhbp;
				lhbp->superBlock  = listbp;
				lastbp = lhbp;
			}
			else if (curType != texts[l]->listType
				  && curHint != texts[l]->listHint
				  && indent == texts[l]->indent) {
				/* Encountered another type of list	*/
				/* Check previous list block		*/
				break;
			}
			else if (texts[l]->indent < indent) {
				break;
			}
			blankPassed = 0;
		}
		else if (texts[l]->block && texts[l]->block->hinted) {
			/* Do Nothing	*/
		}
		else if (!texts[l]->blank
			 && texts[l]->indent < indent) {
			/* End of the indented block	*/
			/* Check previous list block	*/
			break;
		}
		else if (texts[l]->block) {
			if (texts[l]->block->type == TB_SECNUM)
				break;
			if (texts[l]->block->type == TB_LISTHD)
				break;
		}
		/*
		 * ex.
		 *	1) list1
		 *	this is the continuation	<- don't break
		 *
		 *	2) list2
		 *					<- End of List
		 *	this is NoT the continuation	
		 */
		else if (texts[l]->blank)
			blankPassed = 1;
		else if (blankPassed && texts[l]->indent <= indent)
			break;
	}
	if (checkIfList(listbp)
	    /*
	     * Number/Alpha List should begin with 1
	     */
	    && curType <= L_ENUM_MAX
	    && (texts[begin]->listNum != 1)) {
		MSG1("\nWarning[%d] list not start with 1 or A\n", begin);
		MSG1("\t%s\n",codeCvt(texts[begin]->body));
	}
}
/*
 * Check if "toriaezu" marked block is list or not.
 *	(elements >= 2 && list head ni mieru
 *	or element == 1 && list head ni chigainai)
 * If determined as not a list free blocks
 *	(else region end is determined later)
 */
checkIfList(lbp)
struct textBlock *lbp;
{
	struct	textBlock *lhbp;
	int	nitems;
	int	allSingleLine = 1;	/* All items are 1 line	*/
	int	goodlooking = 0;	/* for Description list	*/
	int	rend;
	DBG1(7, "checkIfList (%d)\n", lbp->rbegin);
	for (lhbp = lbp->nextBlock, nitems = 0; lhbp;
	     lhbp = lhbp->nextBlock, nitems++) {
		if (lhbp->nextBlock && (lhbp->nextBlock->rbegin - lhbp->rbegin > 1))
			allSingleLine = 0;
		if (lhbp->nextBlock
		    && (lhbp->nextBlock->rbegin - lhbp->rbegin > 1)
		    && (texts[lhbp->rbegin + 1]->indent >
			texts[lhbp->rbegin]->indent))
			/* Following line is indented	*/
			goodlooking++;
		else if (prevLine(lhbp->rbegin)->blank)
			/* Previous Line blank		*/
			goodlooking++;
	}
	if ((nitems <= 1 && texts[lbp->rbegin]->pListHead < DEFINITELY)
	    || (texts[lbp->rbegin]->listType == L_DLIST
		&& allSingleLine == 0 &&  goodlooking * 4 < nitems)) {
		/* Not a list.  Do nothing	*/
		for (lhbp = lbp; lhbp; lhbp = lhbp->nextBlock)
			free((char *)lhbp);
		return 0;
	}
	for (lhbp = lbp->nextBlock; lhbp; lhbp = lhbp->nextBlock) {
		texts[lhbp->rbegin]->block = lhbp;
		rend = lhbp->rend;
	}
	/* Overwrite first line of the list	*/
	texts[lbp->rbegin]->block = lbp;
	MSG2("%d-%d ", lbp->rbegin, rend - 1);
	return 1;
}
/*
 * Expand list head with following plain text
 */
expandListHead(lhbp)
struct	textBlock *lhbp;
{
	struct	textBlock *nextTbp;
	int	l;
	int	indent1, indent2, indentFb, indentFe;

	if (texts[lhbp->rbegin]->length * 2 < rightMargin)
		/* too Short	*/
		return 0;
	if ((nextTbp = texts[lhbp->rend]->block)
	    && (nextTbp->type == TB_PLAIN)) {
		indent1  = texts[lhbp->rbegin]->indent;
		indent2  = listSecBody(texts[lhbp->rbegin])
			- texts[lhbp->rbegin]->body;
		indentFb = texts[nextTbp->rbegin]->indent;
		indentFe = texts[nextTbp->rend - 1]->indent;
		
		if (indentFb ==	indentFe
		    && indent1 <= indentFb && indentFb <=indent2
		    && indent1 <= indentFe && indentFe <=indent2) {
			for (l = lhbp->rend;
			     l < nextTbp->rend; l++)
				texts[l]->block = lhbp;
			lhbp->rend = nextTbp->rend;
			free((char *)nextTbp);
		}
	}
}
/*
 * Fix list region
 *  ex.
 *	1) list1	      <-+ list region
 *		sub-bock	|
 *	2) list2		|
 *		sub-bock      <-+
 */
fixListEnd(lbp)
struct textBlock *lbp;
{
	struct	textBlock *lhbp;
	int	allSingleLine = 1;
	int	items = 0;
	int	lnum;
	for (lhbp = lbp->nextBlock; lhbp->nextBlock; lhbp = lhbp->nextBlock) {
		items++;
		if (lhbp->rend != lhbp->nextBlock->rbegin)
			allSingleLine = 0;
		expandListHead(lhbp);
		fixListRegion(lhbp->rend, lhbp->nextBlock->rbegin);
	}
	if (lhbp == lbp->nextBlock)
		allSingleLine = 0;
	/* Last item of the list	*/
	if (items < 2 || !allSingleLine)
		expandListHead(lhbp);
	lnum = lhbp->rend;
	while (lnum < textLines) {
		if (texts[lnum]->block
		    && minIndentBlock(texts[lnum]->block)
		    <= texts[lbp->rbegin]->indent)
			break;
		if (texts[lnum]->block && texts[lnum]->block->type == TB_LIST)
			lnum = fixListEnd(texts[lnum]->block);
		else if (texts[lnum]->block)
			lnum = texts[lnum]->block->rend;
		else
			lnum++;
	}
	lbp->rend = lnum;
	return lnum;
}
fixListRegion(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end;) {
		if (texts[l]->block && texts[l]->block->type == TB_LIST) {
			l = fixListEnd(texts[l]->block);
		}
		else
			l++;
	}
}
list(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end; l++) {
		if (texts[l]->block == NULL
		    && texts[l]->pListHead != NEVER)
			findListHead(l, end);
	}
}

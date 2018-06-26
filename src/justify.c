/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: justify.c,v 2.9 1994/04/19 10:16:43 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
/*
 * Justification
 *	"centering"  or "justify to right "
 */
indType(begin, end)
int	begin;
int	end;
{
	register struct text	*textp;
	int	l;
	int	rightJstfy = 1;
	int	centering  = 1;
	int	sameIndentLines = 0;
	int	textCenter, textRight, margin;
	int	topIndent,  rightSpace;
	topIndent  = texts[begin]->indent;
	textCenter = rightMargin / 2;
	textRight  = rightMargin * 8 / 10;
	margin     = rightMargin / 8;
	if (!prevLine(begin)->blank || !nextLine(end - 1)->blank)
		return 0;
	for (l = begin; l < end; l++) {
		if ((textp = texts[l])->block)
			return 0;
		if (textp->indent <= topIndent
		    && textp->indent >= topIndent - 2)
			sameIndentLines++;
		if (textp->indent < textCenter
		    || textp->length < textRight)
			rightJstfy = 0;
		rightSpace = textp->indent
			+ textp->length - rightMargin;
		if (!textp->indent || rightSpace < -margin || rightSpace > margin)
			centering = 0;
		if (!rightJstfy && !centering)
			return IND_INDENT;
	}
	if (centering 
	    && topIndent > rightMargin / 4
	    && (end - begin == 1 || sameIndentLines * 3 < (end - begin) * 2))
	    return IND_CENTER;
	if (rightJstfy)
		return IND_RIGHT;
	return IND_INDENT;
}
/*
 * If given text block is justified, register block attribute as
 * right/center justified.
 */
markIfJustified(begin, end)
int	begin;
int	end;
{
	int	l;
	struct	textBlock	*tbp;
	DBG2(9, "markIfJustified (%d-%d)\n", begin, end);
	switch (indType(begin, end)) {
	    case IND_INDENT:
		return 0;
	    case IND_RIGHT:
		tbp = newTextBlock(begin, end, TB_RIGHT);
		break;
	    case IND_CENTER:
		tbp = newTextBlock(begin, end, TB_CENTER);
		break;
	    default:
		return 0;
	}
	for (l = begin; l < end; l++)
		texts[l]->block = tbp;
}
justified(begin, end)
int	begin;
int	end;
{
	int	rstat;
	int	l, rbegin;
	DBG2(8, "justified (%d-%d)\n", begin, end);
	rstat = 0;
	rbegin = begin;
	for (l = begin; l <= end; l++)
		if (rstat) {
			if (l >= end
			    || texts[l]->blank
			    || texts[l]->block != NULL) {
				markIfJustified(rbegin, l<end?l:end);
				rstat = 0;
			}
		}
		else {
			if (l >= end)
				break;
			else if (!texts[l]->blank
				 && texts[l]->block == NULL) {
				rbegin = l;
				rstat = 1;
			}
		}
}

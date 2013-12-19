/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: plaintxt.c,v 2.12 1994/04/19 10:16:59 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
/*
 * Quoted region. 
 *	>> quoted
 *	>> lines
 */
quotedText(begin, end)
int	begin;
int	end;
{
	int	l1, l2, l3;
	char	topchar, secondchar;
	struct	textBlock *tbp;
	DBG2(7, "markIfQuote (%d-%d)\n", begin , end);
	for (l1 = begin; l1 < end - 1; 	l1++) {
		secondchar = 0;
		if (texts[l1]->block)
			continue;
		topchar    = *texts[l1]->body;
		secondchar = *(texts[l1]->body+1);
		if (topchar != '>' && topchar != ':')
			continue;
		for (l2 = l1 + 1; l2 < end; l2++) {
			if (texts[l2]->block || texts[l2]->indent != 0)
				break;
			if (*texts[l2]->body != topchar
			    || *(texts[l2]->body+1) != secondchar)
				break;
		}
		if (l2 > l1 + 1 /* More than 3 lines	*/ ||
		    (prevLine(l1)->blank && texts[l2]->blank &&
		     topchar == '>' && secondchar == '>')) {
			tbp = newTextBlock(l1, l2, TB_QUOTE);
			MSG2("%d-%d ", l1, l2 - 1);
			for (l3 = l1; l3 < l2; l3++)
				texts[l3]->block = tbp;
		}
		l1 = l2 - 1;
	}
}

/*
 * Plain text
 *	block is separated at each indented line.
 *	  - - - - - - <- top of plain text block

 naturl
 *	- - - - - - - 
 *	- - - - - - - 
 *	  - - - - - - <- also top of plain text block
 *	- - - - - - - 
 *	- - - - - - - 
 */
markAsPlain(begin, end)
int	begin;
int	end;
{
	int	l, indent;
	struct	textBlock *tbp;
	DBG2(7, "markAsPlain (%d-%d)\n", begin , end);
	indent = texts[begin]->indent;
	tbp = newTextBlock(begin, -1, TB_PLAIN);
	for (l = begin; l < end; l++) {
		if (texts[l]->blank || texts[l]->block
		    || texts[l]->indent <= indent - MIN_INDENT
		    || texts[l]->indent > indent)
			return;
		if (texts[l]->indent < indent)
			indent = texts[l]->indent;
		texts[l]->block = tbp;
		tbp->rend = l + 1;
	}
	return;
}
plainText(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end; l++) {
		if (texts[l]->block == NULL
		    && !texts[l]->blank)
			markAsPlain(l, end);
	}
}

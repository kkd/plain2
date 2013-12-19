/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: spacing.c,v 2.10 1994/04/19 10:17:05 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include <ctype.h>
#include "plain2.h"
/*
 * Find newpage (^L) and open space (Large blank area)
 */
spacing(begin, end)
int	begin;
int	end;
{
	int	l;
	int	blanks = 0;
	struct	textBlock *tbp, *prevTbp = NULL;
	pageBp = NULL;
	DBG0(7, "spacing\n");
	for (l = begin; l < end; l++) {
		if (texts[l]->blank) {
			blanks++;
		}
		else {
			if (*texts[l]->body == 12) {
				/* new page (^L == 12)	*/
				tbp = newTextBlock(l, l+1, TB_PAGE);
				if (prevTbp)
					prevTbp->nextBlock = tbp;
				else
					pageBp = tbp;
				prevTbp = tbp;
				MSG1("%d ", l);
				texts[l]->length = 0;
				texts[l]->blank  = 1;
				texts[l]->block = tbp;
			}
			else if (blanks > MIN_SPACING) {
				/* Preparing	*/
				struct	textBlock *tbp;
				int	i;
				tbp = newTextBlock(l - blanks, l, TB_SPACE);
				MSG2("%d-%d ", l - blanks, l - 1);
				DBG2(3, "spacing %d-%d\n", l-blanks, l);
				for (i = l - blanks; i < l; i++)
					texts[i]->block = tbp;
			}
			blanks = 0;
		}
	}
}

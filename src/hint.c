/*
 * Copyright (C) 1991, NEC Corporation.
 */
#ifndef lint
static char rcsid[]=
	"$Id: hint.c,v 2.10 1994/04/19 10:16:41 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
/*
 * Hint to determine the type of text block
 */
struct	hintStr {
	char	*beginstr;
	char	*endstr;
	short	type;
} hintStr[] = {
#ifdef	PICTURE
	{ "[[P", "]]P", TB_PICTURE },
#endif
	{ "[[T", "]]T", TB_TABLE },
	{ "[[E", "]]E", TB_EXAMPLE },
	{ "[[R", "]]R", TB_RAW },
	{ "[[N", "]]N", TB_CENTER },
	{ "[[A", "]]A", TB_RIGHT },	/* Adjust right */
	{ "[[C", "]]C", TB_COMMENT },	/* comment out	*/
	{ "", "", 0}};
checkIfHint(s, which)
char	*s;
int	which;
{
	struct	hintStr		*hp;
	hp = hintStr;
	while (*hp->beginstr) {
		if (which == 0) {
			if (strncmp(s,  hp->beginstr,
				    strlen(hp->beginstr)) == 0) {
				return hp->type;
			}
		}
		else {
			if (strncmp(s,  hp->endstr,
				   strlen(hp->endstr)) == 0) {
				return hp->type;
			}
		}
		hp++;
	}
	return NULL;
}
hint(begin, end)
int	begin;
int	end;
{
	int	i, l, rbegin;
	int	tbType;
	struct	textBlock	*tbp;
	l  = begin;
	while  (l < end) {
		if (tbType = checkIfHint(texts[l]->body, 0)) {
			rbegin = l;
			l++;
			while (l < end) {
				if (checkIfHint(texts[l]->body, 1) == tbType){
					l++;
					tbp = newTextBlock(rbegin, l, tbType);
					tbType = 0;
					tbp->hinted = 1;
					for (i = rbegin; i < l; i++)
						texts[i]->block = tbp;
					break;
				}
				else
					l++;
			}
			if (tbType) {
				/* Can't find  end of hint region*/
				fprintf(stderr, "WARNING: can't find matching %s at line %d\n", texts[rbegin]->body, rbegin);
			}
				
		}
		else
			l++;
	}
}

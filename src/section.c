/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: section.c,v 2.10 1994/04/19 10:17:04 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include <ctype.h>
#include "plain2.h"
#include "macro.h"
/*
 * Section number match
 *	2.4.	<-+ level 2
 *	2.4.1   <-
 */
secNumMatch(s1, s2, d)
char	*s1;
char	*s2;
int	d;
{
	int	n1, n2;
	if (d == 0)
		return 1;
	s1 = strNum(s1, &n1);
	s2 = strNum(s2, &n2);
	if (n1 != n2)
		return 0;
	else
		return secNumMatch(s1, s2, d - 1);
}
/*
 * Section numbers are continuous
 *	1.3.5	<- continuous
 *	1.3.6
 */ 
secNumCont(s1, s2, d)
char	*s1;
char	*s2;
int	d;
{
	int	n1, n2;
	s1 = strNum(s1, &n1);
	s2 = strNum(s2, &n2);
	if (d == 1) {
		if (n2 == n1 + 1)
			return 1;
		else
			return 0;
	}
	return secNumCont(s1, s2, d - 1);
}
secNumIsOne(s1, d)
char	*s1;
int	d;
{
	int	n1;
	s1 = strNum(s1, &n1);
	if (d == 1) {
		if (n1 == 1)
			return 1;
		else
			return 0;
	}
	return secNumIsOne(s1, d - 1);
	
}
/*
 * Two text lines have continuous section numbers
 */
secCnctable(cur, prev)
struct	text	*cur;
struct	text	*prev;
{
	int	depth1, depth2;

	if (prev == NULL)
		return secNumIsOne(cur->body + cur->indent, 1);

	depth1 = prev->secDepth;
	depth2 = cur->secDepth;
	
	if (depth1 == depth2) {
		/* depth - 1 number is same	*/
		if (!secNumMatch(prev->body + prev->indent,
				 cur->body + cur->indent, depth1 - 1))
			return 0;
		/* last number is continuous	*/
		if (!secNumCont(prev->body + prev->indent,
				cur->body + cur->indent, depth1))
			return 0;
	}
	else if (depth1 < depth2) {
		/* depth1 number is same	*/
		if (!secNumMatch(prev->body + prev->indent,
				 cur->body + cur->indent, depth1))
			return 0;
		/* depth2 last number is "1"	*/
		if (!secNumIsOne(cur->body + cur->indent, depth2))
			return 0;
	}
	else /* depth1 > depth2 */{
		/* depth2 - 1 number is same	*/
		if (!secNumMatch(prev->body + prev->indent,
				 cur->body + cur->indent, depth2 - 1))
			return 0;
		/* depth2 number is continuous	*/
		if (!secNumCont(prev->body + prev->indent,
				cur->body + cur->indent, depth2))
			return 0;
	}
	return 1;
}
/*
 */
section(begin, end, phase)
int	begin;
int	end;
int	phase;
{
	int	l;
	char	p;
	struct	text	*prevSectxt = NULL;
	struct	textBlock	*tbp;
	DBG2(7, "section (%d-%d)\n", begin, end);
	for (l = begin; l < end; l++) {
		if (texts[l]->block) {
			if (phase == 1 && texts[l]->block->type == TB_SECNUM)
				prevSectxt = texts[l];
			l = texts[l]->block->rend - 1;
			continue;
		}
		if (texts[l]->pSecNum == NEVER || !prevLine(l)->blank)
			continue;
		if (nextLine(l)->blank)
			p = DEFINITELY;
		else
			p = texts[l]->pSecNum;

		if (p == AMBIGUOUS && !phase
		    && !(prevSectxt && secCnctable(texts[l], prevSectxt)))
			continue;
		MSG1("%d ", l);
		if (!secCnctable(texts[l], prevSectxt)) {
			MSG1("\nWarning[%d] Section number\n", l);
			if (prevSectxt)
				MSG1("\t%s\n", codeCvt(prevSectxt->body));
			MSG1("\t%s\n", codeCvt(texts[l]->body));
		}
		tbp = newTextBlock(l, l + 1, TB_SECNUM);
		texts[l]->block = tbp;
		prevSectxt      = texts[l];
	}
}
/*
 * Initiate section number
 */
setSnumFor(textp)
struct	text	*textp;
{
	char	*snumStr;
	int	i, num;
	snumStr = textp->body + textp->indent;
	for (i = 1; i <= textp->secDepth; i++){
		snumStr = strNum(snumStr, &num);
		if (num != 1
		    || textp->secDepth != 1) {
			if (i == textp->secDepth)
				num--;
			if (putMacro(M_SETSEC_1 + i - 1, num) < 0)
				putMacro(M_SETSEC, i, num);
		}
	}
}
outputSetSnum(begin, end)
int	begin;
int	end;
{
	int	l;
	for (l = begin; l < end; ) {
		if (texts[l]->block) {
			if (texts[l]->block->type == TB_SECNUM) {
				setSnumFor(texts[l]);
				return 0;
			}
			l = texts[l]->block->rend;
		}
		else
			l++;
	}
}
/*
 * Appendix
 */
appendix(begin, end)
int	begin;
int	end;
{
	int	l, apnum;
	int	hlen;
	register struct	text	*textp;
	DBG2(7, "appendix (%d-%d)\n", begin, end);
	for (l = begin; l < end; l++) {
		textp = texts[l];
		if (textp->block) {
			l = textp->block->rend - 1;
			continue;
		}
		if (textp->blank
		    || (!indentedSecnum && textp->indent != 0)
		    || !prevLine(l)->blank
		    || !nextLine(l)->blank)
			continue;
		if ((hlen =
		     checkIfAppendix(textp->body + textp->indent, &apnum))
		    && prevLine(l)->blank) {
			textp->headLen = hlen;
			textp->block = newTextBlock(l, l + 1, TB_APPENDIX);
		}
	}
}

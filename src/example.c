/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: example.c,v 2.10 1994/04/19 10:16:20 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
/*
 * Example Block
 * 	"non-fill" , "constant-width"
 */
markIfExample(begin, end)
int	begin;
int	end;
{
	int	lengthSum = 0, spaceSum = 0, jcodeSum = 0;
	short	aveLength, spacePercent, jcodePercent;
	int	l, totalLines;
	int	indent;
	DBG2(7,"markIfExample (%d-%d)\n", begin, end);
	indent	   = minIndent(begin, end);
	if ((totalLines = end - begin) == 0)
		return 1;
	for (l= begin; l < end; l++) {
		if (texts[l]->block != NULL)
			return 0;
		if (!texts[l]->blank){
			lengthSum += texts[l]->length - indent;
			spaceSum  += texts[l]->spaces;
			jcodeSum  += texts[l]->japanese;
		}
		else
			totalLines--;
	}
	if (totalLines == 0)
		return 1;
	aveLength    = lengthSum      / totalLines ;
	spacePercent = spaceSum * 100 / lengthSum;
	jcodePercent = jcodeSum * 100 / totalLines;
	if (ExampleCheck(begin, end, aveLength, spacePercent,
			 jcodePercent, totalLines)) {
		struct	textBlock *tbp;
		MSG2("%d-%d ",begin, end - 1);
		tbp = newTextBlock(begin, end, TB_EXAMPLE);
		for (l = begin; l < end; l++)
			texts[l]->block = tbp;
		return 1;
		}
	return 0;
}
/*
 * Judge if this region is example or not.
 */
ExampleCheck(begin, end, length, spaces, jcode, lines)
int	begin;
int	end;
int	length;		/* average of text length	*/
int	spaces;		/* percentage of space characters	*/
int	jcode;		/* percentage of Japanese text lines	*/
int	lines;		/* number of lines		*/
{
	int	eval;
	if (japaneseText) {
		if (lines == 1 && texts[begin]->japanese)
			/* Ignore text length if lines == 1.
			 */
			eval = spaces * 4 - jcode/2;
		else
			eval = 2000/length + spaces * 4 - jcode/2;
	}
	else {
		/* English texts tends to have many spaces	 */
		if (lines == 1)
			eval = 1000/length + spaces * 4 - 30;
		else
			eval = 2400/length + spaces * 4 - 30;
	}
	DBG1(7, "Example=%d", eval);
	DBG3(7, "(l:%d, s:%d, j:%d)\n", length, spaces, jcode);
	return ((max(eval, 0) + examFactor - 50) * examFactor >= 5000);
}

/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 */
/*
 * modify by k-chinen@is.aist-nara.ac.jp, 1994
 */
#ifndef lint


#endif

#include <stdio.h>
#include "plain2.h"
#include "picture.h"
#include "macro.h"

#ifdef	MSDOS
/* for Turbo-C(V2) BUG	*/
#define bodyStr(textp)  (textp)->body + ((textp)->blank? 0: (textp)->indent)
#else
#define	bodyStr(textp)	((textp)->blank\
			 ?(textp)->body\
			 :(textp)->body+(textp)->indent)
#endif

/*
 * Definition of Output driver routines.
 */
/***** roff *****/
int	roffPlain(),		roffExample(),
	roffListBlock(),	roffDlistItem(),
	roffListItem(),		roffRawText(),	roffSpace(),
	roffCapsule(),		roffComment(),
#ifdef	PICTURE
	roffPictureBlock(),	roffPicLine(),	roffPicArc(),
	roffPicText(),		roffLineWidth(),
#endif
	roffTable(),		roffFTitle(),	roffTitle();
char	*roffQuote1(),		*roffQuote2();


/***** TeX *****/
int	texPlain(),		texExample(),
	texListBlock(),	texDlistItem(),
	texListItem(),		texRawText(),	texSpace(),
	texCapsule(),		texComment(),
#ifdef	PICTURE
	texPictureBlock(),	texPicLine(),	texPicArc(),
	texPicText(),		texLineWidth(),
#endif
	texTable(),		texFTitle(),	texTitle();
char	*texQuote1(),		*texQuote2();

#ifdef HTML
/***** HTML *****/
int     htmlPlain(),            htmlExample(),
        htmlListBlock(),        htmlDlistItem(),
        htmlListItem(),         htmlRawText(),  htmlSpace(),
        htmlCapsule(),          htmlComment(),
#ifdef  PICTURE
        htmlPictureBlock(),     htmlPicLine(),  htmlPicArc(),
        htmlPicText(),          htmlLineWidth(),
#endif
        htmlTable(),            htmlFTitle(),   htmlTitle();
char    *htmlQuote1(),          *htmlQuote2();
#endif

struct outDev roffPut = {
	roffPlain,		roffExample,
	roffListBlock,	roffDlistItem,
	roffListItem,		roffRawText,	roffSpace,
	roffCapsule,		roffComment,
#ifdef	PICTURE
	roffPictureBlock,	roffPicLine,	roffPicArc,
	roffPicText,		roffLineWidth,
#endif
	roffTable,		roffFTitle,	roffTitle,
	roffQuote1,		roffQuote2,
};

struct outDev texPut = {
	texPlain,		texExample,
	texListBlock,		texDlistItem,
	texListItem,		texRawText,	texSpace,
	texCapsule,		texComment,
#ifdef	PICTURE
	texPictureBlock,	texPicLine,	texPicArc,
	texPicText,		texLineWidth,
#endif
	texTable,		texFTitle,	texTitle,
	texQuote1,		texQuote2,
};

#ifdef HTML
struct outDev htmlPut = {
        htmlPlain,              htmlExample,
        htmlListBlock,          htmlDlistItem,
        htmlListItem,           htmlRawText,    htmlSpace,
        htmlCapsule,            htmlComment,
#ifdef  PICTURE
        htmlPictureBlock,       htmlPicLine,    htmlPicArc,
        htmlPicText,            htmlLineWidth,
#endif
        htmlTable,              htmlFTitle,     htmlTitle,
        htmlQuote1,             htmlQuote2,
};
#endif


#ifdef HTML
/*
 * Output text in (roff, TeX or HTML) format.
 */
#else
/*
 * Output text in (roff or TeX) format.
 */
#endif

struct	inline_mark {
	char	il_mark;
	int	il_type;
}inline_mark[] = {
	{'r', IL_RAW},
	{'f', IL_FOOTNOTE},
	{'x', IL_REFERENCE},
	{'b', IL_BOLD},
	{'i', IL_INDEX},
};

inlineBegin(s, type)
char	*s;
int	*type;
{
	char	*t;
	int	i;
	int	offset = 0;
	while(*s) {
		if ((t = index(s, INLINE_BEGIN[0])) == NULL || *(t+1) == '\0')
			return -1;
		if (*(t + 1) == INLINE_BEGIN[1]) {
			for (i = 0; i < sizeof(inline_mark)/sizeof(struct inline_mark);
			     i++) {
				if (*(t + 2) == inline_mark[i].il_mark) {
					*type = inline_mark[i].il_type;
					return t - s + offset;
				}
			}
		}
		offset += t - s + 1;
		s = t + 1;
	}
	return 0;
}
inlineEnd(s)
char	*s;
{
	char	*t;
	int	offset = 0;

	while(*s) {
		if ((t = index(s, INLINE_END[0])) == NULL)
			return -1;
		if (*(t + 1) == INLINE_END[1]
		    && *(t + 2) != INLINE_END[0])	/*XXX*/
			return t - s + offset;
		offset += t - s + 1;
		s = t + 1;
	}
	return 0;
}
inlineEndRegion(begin, end)
int	begin;
int	end;
{
	while (begin < end) {
		if (inlineEnd(texts[begin++]->body) != -1)
			return begin;
	}
	return 0;
}
outAttribute(attr, begin)
int	attr;
int	begin;
{
	switch (attr) {
	    case IL_RAW:
		break;
	    case IL_FOOTNOTE:
		if (begin)
			putMacro(M_FOOTN_BEGIN);
		else
			putMacro(M_FOOTN_END);
		break;
	    case IL_REFERENCE:
		if (begin)
			putMacro(M_REFER_BEGIN);
		else
			putMacro(M_REFER_END);
		break;
	    case IL_BOLD:
		if (begin)
			putMacro(M_BOLD_BEGIN);
		else
			putMacro(M_BOLD_END);
		break;
	    case IL_INDEX:
		if (begin)
			putMacro(M_INDEX_BEGIN);
		else
			putMacro(M_INDEX_END);
		break;
	    default:
		fprintf(stderr, "PANIC(unknown inline type)\n");
		exit(2);
	}
}

textOuput(begin, end, s)
int	begin;
int	end;
char	*s;
{
	int	col1, col2, fend;
	int	inline_attr;
	static char	buf[MAX_LINE_LEN];
	while (1) {
		if (s == NULL || *s == '\0') {
			PRINTED(begin);
			if (++begin < end)
				s = bodyStr(texts[begin]);
			else
				break;
		}
		if (inlineHint 
		    && ((col1 = inlineBegin(s, &inline_attr)) != -1)
		    && ((col2 = inlineEnd(s + col1 + INLINE_END_LEN)) != -1)) {
 			if (col1 != 0) {
				(void)strncpy(buf, s, col1);
				buf[col1] = '\0';
				(*put->plain)(buf, IL_PLAIN, 1);
			}

			outAttribute(inline_attr, BEGIN);

			(void)strncpy(buf, s + col1 + INLINE_BEGIN_LEN, col2);
			buf[col2] = '\0';
			(*put->plain)(buf, inline_attr, 1);

			outAttribute(inline_attr, END);

			s += col1 + col2 + INLINE_BEGIN_LEN  + INLINE_END_LEN -1;
		}
		else if (inlineHint
			 && col1 != -1
			 && (fend = inlineEndRegion(begin + 1, end))) {
			if (col1 != 0) {
				(void)strncpy(buf, s, col1);
				buf[col1] = '\0';
				(*put->plain)(buf, IL_PLAIN, 1);
			}
			outAttribute(inline_attr, BEGIN);
			(*put->plain)(s + col1 + INLINE_BEGIN_LEN,
				      inline_attr, 1);
			PRINTED(begin);
			begin++;
			while (begin < fend - 1) {
				(*put->plain)(bodyStr(texts[begin]),
					      inline_attr, 1);
				PRINTED(begin);
				begin++;
			}
			s = texts[begin]->body;
			col1 = inlineEnd(s);
			(void)strncpy(buf, s, col1);
			buf[col1] = '\0';
			(*put->plain)(buf, inline_attr, 1);
			outAttribute(inline_attr, END);
			s += col1 + INLINE_END_LEN - 1;
		}
		else {
			(*put->plain)(s, IL_PLAIN, 1);
			PRINTED(begin);
			if (++begin < end)
				s = bodyStr(texts[begin]);
			else
				break;
			continue;
		}
	}
}
#define	MAX_LIST_LEVEL	8
static	int	listLevel  = 0;
static	int	listIndent = 0;
listOutput(lbp)
struct textBlock *lbp;
{
	char	dscr[MAX_LINE_LEN];
	struct	textBlock *lhbp;
	char	*firstLine;
	int	lnum, rend;
	int	lastIndent;

	lastIndent = listIndent;
	listIndent = texts[lbp->rbegin]->indent;
	listLevel++;
	(*put->listBlock)(BEGIN, listLevel, texts[lbp->rbegin]->listType,
			  texts[lbp->rbegin]->listHint);
	for (lhbp = lbp->nextBlock; lhbp != NULL; lhbp = lhbp->nextBlock) {
		lnum = lhbp->rbegin;
		firstLine = NULL;
		/* output first block (list head block)	*/
		if (texts[lnum]->listType == L_DLIST) {
			(void)strncpy(dscr, bodyStr(texts[lnum]),
				texts[lnum]->headLen);
			dscr[texts[lnum]->headLen] = '\0';
			(*put->dlistItem)(listLevel, dscr,
					  *listSecBody(texts[lnum]) != '\0');
			if (*listSecBody(texts[lnum]) != '\0')
				firstLine = listSecBody(texts[lnum]);
		}
		else {
			(*put->listItem)(listLevel);
			firstLine = listSecBody(texts[lnum]);
		}
		textOuput(lnum, lhbp->rend, firstLine);
		/* output rest of the list body	*/
		if (lhbp->nextBlock != NULL) {
			rend = lhbp->nextBlock->rbegin;
		}
		else
			rend = lbp->rend;
		outputRegion(lhbp->rend, rend);
	}
	(*put->listBlock)(END, listLevel, texts[lbp->rbegin]->listType,
			texts[lbp->rbegin]->listHint);
	listLevel--;
	listIndent = lastIndent;
}
capsuleOutput(blockTbp)
struct	textBlock	*blockTbp;
{
	struct	textBlock	*tbp1, *tbp2;
	int	center;
	int	ftType; /* type of the figure/table title	*/
	int	l;

	tbp1 = blockTbp->nextBlock;
	tbp2 = tbp1->nextBlock;
	
	if (tbp1->type == TB_FTITLE) {
		ftType = texts[tbp1->rbegin]->fTitle;
	}
	else /* if (tbp1->type == TB_FTITLE)*/ {
		ftType = texts[tbp2->rbegin]->fTitle;
	}

	center = blockIsCenter(blockTbp);
	(*put->capsule)(BEGIN, ftType, center);

	outputBlock(tbp1, 1);
	for (l = tbp1->rend; l < tbp2->rbegin; l++) {
		if (texts[l]->block == NULL && texts[l]->blank)
			PRINTED(l);
	}
	outputBlock(tbp2, 1);

	(*put->capsule)(END, ftType, center);
}
outputBlock(tbp, capsule)
struct	textBlock *tbp;
int	capsule;
{
	int	i, blockIndent;
	int	begin, end;
	begin = tbp->rbegin;
	end   = tbp->rend;
	if (tbp->hinted) {
		PRINTED(begin);
		PRINTED(end - 1);
		begin++;
		end--;
	}
	switch (tbp->type) {
	    case TB_COMMENT:
		PRINTED2(begin, end);
		(*put->comment)(begin, end);
		break;
	    case TB_PAGE:
		putMacro(M_PAGE);
		PRINTED(begin);
		break;
	    case TB_SPACE:
		(*put->space)(tbp->rend - tbp->rbegin);
		PRINTED2(begin, end);
		break;
	    case TB_APPENDIX:
		{
			static firstTime = 1;
			char *title = listSecBody(texts[begin]);
			if (firstTime) {
				firstTime = 0;
				putMacro(M_APDX_BEGIN);
			}
			putMacro(M_APPENDIX, title);
			if (put == &texPut && crossRefer) {
				texPutLabel(title);
			}
			PRINTED(begin);
		}
		break;
	    case TB_SECNUM:
		{
			int depth  = texts[begin]->secDepth;
			char *title = listSecBody(texts[begin]);
			if (anySection)
				putMacro(M_SECTION_END);
			else
				anySection = 1;
			if (depth < 0)
				depth = 0;
			if (accurateSecnum)
				setSnumFor(texts[begin]);
			if ((depth > M_SECTION - M_SECTION_1)
			    || putMacro(M_SECTION_1 + depth - 1, title) < 0)
				putMacro(M_SECTION, depth, title);
			if (put == &texPut && crossRefer) {
				texPutLabel(title);
			}
			PRINTED(begin);
		}
		break;
	    case TB_RIGHT:
		putIndent(IND_RIGHT, 1);
		goto rightCenter;
	    case TB_CENTER:
		putIndent(IND_CENTER, 1);
	    rightCenter:
		blockIndent = minIndent(begin, end);
		for (i = begin; i < end; i++) {
			PRINTED(i);
			(*put->plain)(bodyStr(texts[i]), IL_PLAIN, 1);
			putMacro(M_NEWLINE);
		}
		putIndent(IND_RESUME, 0);
		break;
	    case TB_FTITLE:
		if (!capsule)
			putIndent(IND_INDENT, texts[begin]->indent);
		(*put->fTitle)(texts[begin]->fTitle,
			       texts[begin]->body
			       + texts[begin]->indent
			       + texts[begin]->headLen, capsule);
		PRINTED(begin);
		if (!capsule){
			if(texts[begin]->indent && put == &texPut){
			 /* この場合、texFTitle()の出力が「\\」で終わっており、
			    次のputIndent()が「\endlist」を出力する直前に
			    「\mbox{}」を入れてunderfull hboxの警告を抑える */
				printf("\\mbox{}");
			}
			putIndent(IND_RESUME, 0);
		}
		break;
	    case TB_QUOTE:
	    case TB_EXAMPLE:
		putMacro(useJverb ? M_JEXAM_BEGIN : M_EXAM_BEGIN, fontSize);
		for (i = begin; i < end; i++) {
			PRINTED(i);
			if (texts[i]->blank)
				(*put->example)(texts[i]->body);
			else
				(*put->example)(texts[i]->body + listIndent);
		}
		putMacro(useJverb ? M_JEXAM_END : M_EXAM_END);
		break;
	    case TB_LISTHD:
		printf("ERROR(List Output%d)%s\n", begin, texts[begin]->body);
		break;
	    case TB_LIST:
		blockIndent = minIndent(begin, end);
		if (listLevel == 0 && blockIndent > 6)
			putIndent(IND_INDENT,
				  blockIndent - 4);
		listOutput(tbp);
		if (listLevel == 0 && blockIndent > 6)
			putIndent(IND_RESUME, 0);
		break;
	    case TB_PLAIN:
		blockIndent = minIndent(begin, end);
		if (listLevel == 0 && blockIndent > 4)
			putIndent(IND_INDENT,
				  blockIndent);
		putMacro(M_PLAIN_BEGIN);
		textOuput(begin, end, bodyStr(texts[begin]));
		putMacro(M_PLAIN_END);
		if (listLevel == 0 && blockIndent > 4)
			putIndent(IND_RESUME, 0);
		break;
	    case TB_CAPSULE:
		capsuleOutput(tbp);
		break;
	    case TB_TABLE:
		{
			int	center;
			int	indent_amount = 0;

			if (!capsule) {
				center = isCenter(begin, end);
				(*put->capsule)(BEGIN, FT_TABLE, center);
				if (!center){
					indent_amount = 
					    texts[begin]->indent / 2;

					if(!crossRefer && put == &texPut &&
					   indent_amount){
					 /* この場合、texCapsule()の出力が
					    「\\」で終わっており、次の
					    putIndent()が「{\list{}…」を出力
					    する直前に「\mbox{}」を入れて
					    underfull hboxの警告を抑える */
					  	printf("\\mbox{}");
					}
					putIndent(IND_INDENT, indent_amount);
				}
			}
			tblOutput(begin, end);
			if (!capsule) {
				if (!center){
					if(put == &texPut && indent_amount){
					 /* この場合、直前のputIndent()の出力
					    が「\\」で終わっており、次の
					    putIndent()が「\endlist」を出力
					    する直前に「\mbox{}」を入れて
					    underfull hboxの警告を抑える */
					  	printf("\\mbox{}");
					}
					putIndent(IND_RESUME, 0);
				}
				(*put->capsule)(END, FT_TABLE, center);
			}
		}
		break;
#ifdef	PICTURE
	    case TB_PICTURE:
		{
			int	center;
			blockIndent = minIndent(begin, end);
			if (!capsule) {
				center = isCenter(begin, end);
				(*put->capsule)(BEGIN, FT_FIGURE, center);
			}
			picOutput(begin, end);
			PRINTED2(begin, end);
			if (!capsule)
				(*put->capsule)(END, FT_FIGURE, center);
		}
		break;
#endif
	    case TB_RAW:
		for (i = begin; i < end; i++) {
			(*put->rawText)(texts[i]->body);
			PRINTED(i);
		}
		break;
	    default:
		fprintf(stderr, "PANIC(unknown in output)\n");
		exit(2);
	}
}
outputRegion(begin, end)
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
			outputBlock(tbp, 0);
			l = tbp->rend;
		}
		else {
			if (!texts[l]->blank)
				printf("undef%d:%s\n", l, texts[l]->body);
			else {
				PRINTED(l);
				if (reflectSpace)
					putMacro(M_BLANK);
			}
			l++;
		}
	}
}
putIndent2(type, d)
int	type;
int	d;
{
	switch	(type) {
	    case IND_CENTER:
		if (d)
			putMacro(M_CENTER_BEGIN);
		else
			putMacro(M_CENTER_END);
		break;
	    case IND_RIGHT:
		if (d)
			putMacro(M_RIGHT_BEGIN);
		else
			putMacro(M_RIGHT_END);
		break;
	    default:
		if (d)
			putMacro(M_INDENT, d);
		else
			putMacro(M_INDENT0, d);
		break;
	}
}
putIndent(type, d)
int	type;
int	d;
{
	static	int lastIndentType = -1;
	static	int lastDepth      = 0;
	if (type == IND_RESUME && lastIndentType != -1) {
		switch	(lastIndentType) {
		    case IND_CENTER:
		    case IND_RIGHT:
			putIndent2(lastIndentType, 0);
			break;
		    default:
			if (lastDepth)
				putIndent2(IND_INDENT, 0);
			break;
		}
		lastIndentType = -1;
	}
	if (type == IND_RESUME)
		return;
	switch	(type) {
	    case IND_CENTER:
	    case IND_RIGHT:
		putIndent2(type, 1);
		break;
	    default:
		if (lastDepth = d)
			putIndent2(IND_INDENT, d);
		break;
	}
	lastIndentType = type;
}
outputDocument()
{
	if (textBegin != 1)
		outputTitle();
	if (put == &roffPut)
		roffPutHeader();
	if (preamble)
		outputSetSnum(textBegin, textLines);
	outputRegion(textBegin, textLines);
}

/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: miscutil.c,v 2.15 1994/04/19 10:16:50 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
#include "kanji.h"
#include <ctype.h>
#ifdef	MSDOS
/*
 * get option letter from argument vector
 */
int	opterr = 1,		/* useless, never set or used */
	optind = 1,		/* index into parent argv vector */
	optopt;			/* character checked for validity */
char	*optarg;		/* argument associated with option */

#define BADCH	(int)'?'
#define EMSG	""
#define tell(s)	fputs(*nargv,stderr);fputs(s,stderr); \
		fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);
getopt(nargc,nargv,ostr)
int	nargc;
char	**nargv,
	*ostr;
{
	static char	*place = EMSG;	/* option letter processing */
	register char	*oli;		/* option letter list index */

	if(!*place) {			/* update scanning pointer */
		if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
		if (*place == '-') {	/* found "--" */
			++optind;
			return(EOF);
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
		if(!*place) ++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':') {		/* don't need argument */
		optarg = NULL;
		if (!*place) ++optind;
	}
	else {				/* need an argument */
		if (*place) optarg = place;	/* no white space */
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
	 	else optarg = nargv[optind];	/* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);			/* dump back option letter */
}
#endif
/*
 * Value of number string(hankaku or Zenkaku)
 * return rest of the string
 */
char	*
strNum(str, rval)
char	*str;
int	*rval;
{
	*rval = 0;
	while(1) {
		if (
#ifdef	KANJI
		    !isZenkaku(str) &&
#endif
		    isdigit(*str)){
			*rval = *rval * 10 + (*str - '0');
			str++;
		}
#ifdef	KANJI
		else if (maybeZenkakuNum(str)) {
			/* Zenkaku Number	*/
			*rval = *rval * 10 + ZenkakuNumVal(str);
			str += 2;
		}
#endif
		else {
			/* search Next number	*/
			while (*str) {
#ifdef	KANJI
				if (maybeZenkakuNum(str))
					break;
				if (isZenkaku(str))
					str += 2;
				else
#endif
					if (isdigit(*str))
						break;
					else
						str++;
			}
			return str;
		}
	}
}
#ifdef	KANJI
char	*
euc2jisStr(str, jstr)
char	*str;
char	*jstr;
{
	char	*t = jstr;
	int	inKanji = 0;
	while(*str) {
		if (t >= jstr + MAX_LINE_LEN) {
			fprintf(stderr, "PANIC(buffer overflow in jis convert)\n");
			exit(2);
		}
		if (!inKanji && isZenkaku(str)) {
			(void)strncpy(t,"\033$B", 3);
			t += 3;
			inKanji = 1;
		}
		else if (inKanji && !isZenkaku(str)) {
			(void)strncpy(t,"\033(B", 3);
			t += 3;
			inKanji = 0;
		}
		*t++ = *str++ & 0x7f;
		if (t > jstr + MAX_LINE_LEN) {
			fprintf(stderr, "PANIC(buffer overflow in jis convert)\n");
			exit(2);
		}
	}
	if (inKanji) {
		(void)strncpy(t,"\033(B", 3);
		t += 3;
	}
	*t = '\0';
	return jstr;
}
char	*
euc2sftjStr(str, sjstr)
char	*str;
char	*sjstr;
{
	register char c1, c2;
	char	*t = sjstr;
	while(*str) {
		if (isZenkaku(str)) {
			c2 = (*str++) & 0x7f;
			c1 = (*str++) & 0x7f;
			*t++ =  ((c2 - 1) >> 1) + ((c2 <= 0x5e) ? 0x71 : 0xb1);
			*t++ =  c1 + ((c2 & 1) ? ((c1 < 0x60) ? 0x1f : 0x20) : 0x7e);
		}
		else {
			*t++ = *str++;
		}
	}
	*t = '\0';
	return sjstr;
}
#define isSjKanji(c)      (((c)>=0x80 && (c)<0xa0) || ((c)>=0xe0 && (c)<0xfd))
char	*
sftj2eucStr(str, estr)
char	*str;
char	*estr;
{
	int c1, c2;
	char	*t = estr;
	while(*str) {
		c1 = (unsigned char)*str++;
		if (isSjKanji(c1)) {
			c2 = (unsigned char)*str++;
			if (c2 >= 0x9f) {
				*t++ = (c1 >= 0xe0)?
					c1*2 - 0xe0 : c1*2 - 0x60;
				*t++ = c2 + 2;
			} else {
				*t++ = (c1 >= 0xe0)?
					c1*2 - 0xe1 : c1*2 - 0x61;
				*t++ = (c2 >= 0x7f)?
					c2 + 0x60 : c2 + 0x61;
			}
		}
		else {
			*t++ = c1;
		}
	}
	*t = '\0';
	return estr;
}
#endif
char	*
codeCvt(str)
char	*str;
{
#ifdef	KANJI
#if	INTERNAL_CODE == CODE_EUC
	static	char	cvtBuf[MAX_LINE_LEN];
	if (outputCode == CODE_JIS)
		return euc2jisStr(str, cvtBuf);
	else if (outputCode == CODE_SJIS)
		return euc2sftjStr(str, cvtBuf);
	else
		return str;
#else
#if	INTERNAL_CODE == CODE_SJIS
	return str;
#else
unknown code;
#endif
#endif
#else
	return str;
#endif
}
minIndentBlock(tbp)
struct	textBlock	*tbp;
{
	if (tbp->type == TB_LIST) {
		return  texts[tbp->rbegin]->indent;
	}
	return minIndent(tbp->rbegin + tbp->hinted, tbp->rend - tbp->hinted);
}
minIndent(begin, end)
int	begin;
int	end;
{
	register int	i;
	register int	mini, indent;
	mini = texts[begin]->indent;
	for (i = begin; i < end; i++) {
		indent = texts[i]->indent;
		if (!texts[i]->blank && indent < mini)
		if ((mini = indent) == 0)
			return 0;
	}
	return mini;
}
maxLength(begin, end)
int	begin;
int	end;
{
	int	maxl;
	int	i;
	maxl = 0;
	for (i = begin; i < end; i++) {
		if (texts[i]->length > maxl)
			maxl = texts[i]->length;
	}
	return maxl;
}
isCenter(begin, end)
int	begin;
int	end;
{
	int	left, right;
	left = minIndent(begin, end);
	right= rightMargin - maxLength(begin, end);
	if (right < 0)
		return (left <= MIN_INDENT);
	if (left > MIN_INDENT && right < rightMargin/4)
		return 1;
	if ((left * 2) <= (right * 5) &&
	    (left * 5) >= (right * 2))
		return 1;
	return 0;
}
blockIsCenter(tbp)
struct	textBlock	*tbp;
{
	return isCenter(tbp->rbegin, tbp->rend);
}
char	*
listSecBody(textp)
struct	text	*textp;
{
	char	*s;
	s = textp->body + textp->indent + textp->headLen;
	while (*s != '\0') {
		if (*s != ' ')
			break;
		s++;
	}
	return s;
}
textQuoteChar(top, s, trtp)
register char	**top;
register char	*s;
register struct transTable *trtp;
{
	register char	*tto;
	for (; trtp->special_char != 0; trtp++) {
		if (*s == trtp->special_char) {
			for (tto=trtp->trans_to; *tto!= '\0'; tto++)
				*(*top)++ = *tto;
			return;
		}
	}
	*(*top)++ = *s;
}
char	*
textQuote(str, tr_table)
register char	*str;
struct transTable	*tr_table;
{
	static	char	translated[MAX_LINE_LEN];
	char	*t;
	if (rawOutput)
		return str;
	t = translated;
	for(; *str != '\0'; str++) {
#ifdef	KANJI
		if (isZenkaku(str)) {
			*t++ = *str++;
			*t++ = *str;
		}
		else
#endif
			textQuoteChar(&t, str, tr_table);
	}
	*t = '\0';
	return translated;
}
/*
 * Blank line structure (Blank:true, Length:0)
 */
static struct text blank =
{
	NULL, NULL, 1,
	0, 0, 0, 0,
	0
};
struct text *
prevLine(lnum)
int	lnum;
{
	if (lnum <= textBegin)
		return &blank;
	return (texts[lnum-1]);
}
struct text *
nextLine(lnum)
int	lnum;
{
	if (lnum+1 >= textLines)
		return &blank;
	return (texts[lnum+1]);
}
struct textBlock *
prevBlock(lnum, bound)
int	lnum;
int	bound;
{
	while(--lnum >= bound) {
		if (texts[lnum]->block) {
			if (texts[lnum]->block->superBlock)
				return texts[lnum]->block->superBlock;
			else
				return texts[lnum]->block;
		}
	}
	return NULL;
}
struct textBlock *
nextBlock(lnum, bound)
int	lnum;
int	bound;
{
	for( ;lnum < bound; lnum++) {
		if (texts[lnum]->block) {
			if (texts[lnum]->block->superBlock)
				return texts[lnum]->block->superBlock;
			else
				return texts[lnum]->block;
		}
	}
	return NULL;
}
#ifdef	KANJI
/*
 * Character at column "col" in string "s" is kanji character
 */
isKanjiChar(textp, col)
register struct text	*textp;
register short	col;
{
	register char	*s;
	if (col > textp->length || col < textp->indent)
		return 0;
	s = textp->body + textp->indent;
	col -= textp->indent;
	for (; *s && col > 0; s++, col--) {
		if (isZenkaku(s)) {
			s++;
			col--;
		}
	}
	if (col == 0 && isZenkaku(s))
		return 1;
	return 0;
}
#endif
struct textBlock *
newTextBlock(begin, end, type)
int	begin;
int	end;
int	type;
{
	struct textBlock *tbp;
	tbp = (struct textBlock *)malloc(sizeof(struct textBlock));
	if (tbp == NULL) {
		fprintf(stderr, "PANIC(malloc in newTextBlock)\n");
		exit (2);
	}
	bzero((char *)tbp, sizeof(struct textBlock));
	tbp->rbegin = begin;
	tbp->rend   = end;
	tbp->hinted = 0;
	tbp->type   = type;
	return tbp;
}
#define	S_START		1
#define	S_SPACED	2
#define	S_INDENTED	3
/*
 * phase-0:	1, 2, 3
 * phase-1:	4, 5, 6
 *
 *  +-  + ------------------------
 *  4   | ------------------------
 *  +-  | ------------------------
 *	|	+ ----------------
 *	1	2	3 --------
 *	|	+ ----------------
 *  +-	+ ------------------------
 *  5     ------------------------
 *  +-    ------------------------
 *
 *  +-	 ------------------------
 *  6    ------------------------
 *  +-   ------------------------
 */
applyOnRegion2(begin, end, phase, func)
int	begin;
int	end;
int	phase;
int	(*func)();
{
	register int	l;
	register struct text	*textp;
	int	rstat, indent;
	int	rbegin, ibegin, rend;
	DBG3(8, "applyOnRegion-%d (%d-%d)\n", phase, begin, end);
	rstat = S_START;
	indent = minIndent(begin, end);
	rbegin = begin;
	for (l = begin; l <= end; l++) {
		textp = texts[l];
		switch (rstat) {
		    case S_START:
			DBG1(11, "aor START %d\n", l);
			if (l >= end)
				break;
			else if (!textp->blank
				 && textp->block == NULL) {
				if (textp->indent >= indent + MIN_INDENT) {
					ibegin = l;
					rend = l + 1;
					rstat = S_INDENTED;
				}
				else {
					rbegin = l;
					rstat = S_SPACED;
				}
			}
			break;
		    case S_SPACED:
			DBG2(11, "aor SPACED %d B:%d\n", l, rbegin);
			if (l >= end
			    || textp->blank
			    || textp->block != NULL) {
				if (phase == 1)
					(*func)(rbegin, l<end?l:end);
				rstat = S_START;
			}
			else if (textp->indent >= indent + MIN_INDENT) {
				ibegin = l;
				rend = l + 1;
				rstat = S_INDENTED;
			}
			break;
		    case S_INDENTED:
			DBG2(11, "aor INDENTED %d B:%d\n", l, rbegin);
			if (l == end
			    || (!textp->blank
				&& textp->indent < indent + MIN_INDENT)){
				if (phase == 0
				    && !texts[rbegin]->japanese) /* XXX */
					(*func)(rbegin, (l+1)<end?(l+1):end-1);
				if (phase == 0 && (*func)(ibegin, rend) == 0)
					applyOnRegion(ibegin, rend, func);
				rbegin = l;
				rstat = S_SPACED;
			}
			else if (!isBlank(l))
				rend = l + 1;
			else if (!textp->blank
				 && textp->block == NULL)
				return;
			break;
		    default:
			fprintf(stderr, "PANIC(undefined status)%d\n", rstat);
			exit(2);
		}
	}
}
applyOnRegion(begin, end, func)
int	begin;
int	end;
int	(*func)();
{
	applyOnRegion2(begin, end, 0, func);
	applyOnRegion2(begin, end, 1, func);
}
applyOnSpaced(begin, end, func)
int	begin;
int	end;
int	(*func)();
{
	register struct text	*textp;
	register int	l;
	int	rstat;
	int	rbegin;
	DBG2(8, "applyOnSpaced (%d-%d)\n", begin, end);
	rstat = S_START;
	rbegin = begin;
	for (l = begin; l <= end; l++) {
		textp = texts[l];
		switch (rstat) {
		    case S_START:
			if (l >= end)
				break;
			else if (!textp->blank
				 && textp->block == NULL) {
				rbegin = l;
				rstat = S_SPACED;
			}
			break;
		    case S_SPACED:
			if (l >= end
			    || textp->blank
			    || textp->block != NULL) {
				(*func)(rbegin, l<end?l:end);
				rstat = S_START;
			}
			break;
		    default:
			fprintf(stderr, "PANIC(undefined status)%d\n", rstat);
			exit(2);
		}
	}
}
/*
 * join blocks
 */
joinBlocks(begin, end, tbType)
int	begin;
int	end;
int	tbType;
{
	struct textBlock	*tbp;
	struct textBlock	*nextTbp;
	int	l, l2;
	for (l = begin; l < end; l++) {
		if ((tbp = texts[l]->block) && !tbp->hinted 
		    && tbp->type == tbType) {
			nextTbp = texts[tbp->rend]->block;
			if (nextTbp
			    && !nextTbp->hinted && nextTbp->type == tbType){
				/* join it	*/
				DBG4(3, "Join [%d,%d]-[%d,%d]\n",
				     tbp->rbegin, tbp->rend,
				     nextTbp->rbegin, nextTbp->rend);
				for (l2 = tbp->rend;
				     l2 < nextTbp->rend; l2++)
					texts[l2]->block = tbp;
				tbp->rend = nextTbp->rend;
				free((char *)nextTbp);
			}
		}
	}
}

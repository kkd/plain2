/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 * $Id: plain2.h,v 1.5 2001-12-06 18:24:01+09 kenichi Exp $ (NEC)
 */
/*
 * plain2 -- plain tex to {roff, LaTeX} translator
 *
 *		Author:  uchida@ccs.mt.nec.co.jp (Uchida, Akihiro)
 *
 *		Organization: C&C Common Software Development Lab.
 *				NEC Corporation
 *
 *			thanks to: sano@ccs.mt.nec.co.jp (Sano, Susumu)
 *				   saji@ccs.mt.nec.co.jp (Saji, Nobuyuki)
 *		0.0: 18/12/89
 *		1.0: 17/08/90
 *		2.0: 12/12/90
 *		2.2: 31/05/91
 *		2.4: 28/04/92
 *		2.5: 06/08/93
 */
/*
 * modify by k-chinen@is.aist-nara.ac.jp, 1994
 */

/* #define	NEC_CCS			/* NEC C&C Common Software Lab. dependent */

/* #undef	NTT_TEX			/* NTT jlatex	*/ /* no longer needed */

#define	TITLE_SPECIAL

#define	PLAIN2_ENV	"PLAIN2_INIT"
#define	PLAIN2_LIB	"PLAIN2_LIB"

#define	MACRO_LIB	"/usr/lib/plain2"

#define DEFAULT_STY	""	/* default style in TeX output */
/*#define DEFAULT_STY	"a4" */

#define	MIN_INDENT	4	/* minimum indentation to be indented region */
#define	MIN_SPACING	4	/* minimum number of lines to be space region*/
#define	MAX_DSCRLEN	40	/* maximum length for description list	*/

#define	DEF_PAGE_WIDTH	820	/* default page width in pt. (A4 size)	*/
#define MAX_LINE_LEN	1024	/* length of input buffer		*/
#define	ESC	0x1b		/* Escape character	*/
/*
 *	Text Line structure
 */
struct	text {
	struct	textBlock *block;
	char	*body;		/* Pointer to saved text line		*/
	/* outlooks of the line	*/
	short	blank;		/* Blank line				*/
	short	length;		/* Text Length (including leading white)*/
	short	indent;		/* Text Indentation		       	*/
	short	spaces;		/* Number of space			*/
	short	japanese;	/* If Japanese character included	*/
	short	picLines;	/* Number of picture line		*/
	/* result of line analysis	*/
#define	DEFINITELY	2
#define	AMBIGUOUS	1
#define	NEVER		0
	short	pListHead;	/* looks like list head		*/
	short	pSecNum;	/* looks like section number	*/
#define	FT_FIGURE	1
#define	FT_TABLE	2
	short	fTitle;
#define	L_NUMBER	1		/* Number list head	*/
#define	L_SROMAN	2		/* lower Roman Number list head	*/
#define	L_LROMAN	3		/* upper Roman Number list head	*/
#define	L_SALPHA	4		/* Alphabetic(small) list head	*/
#define	L_LALPHA	5		/* Alphabetic(large) list head	*/
#define	L_ENUM_MAX	6	/* type <= L_ENUM_MAX are enum list	*/
#define	L_BULLET	7		/* Bullet list head	*/
#define	L_DASH		8		/* Dash list head	*/
#define	L_DLIST		9		/* Description list 		*/
	short	listType;	/* type of list if listHead==T	*/
#define	LH_PAREN	1		/* both side parenthesis (1)	*/
#define	LH_RPAREN	2		/* right side parenthesis 1)	*/
#define	LH_BRACKET	3		/* both side bracket [1]	*/
#define	LH_RBRACKET	4		/* right side bracket 1]	*/
#define	LH_DOTTED	5		/* both side parenthesis  1.	*/
	short	listHint;	/* more info for list	*/
	short	headLen;	/* length of the Head (list, secnum)	*/
	short	secDepth;	/* depth of section	*/
	short	listNum;	/* value of list  (also used for ftitle) */
#ifdef	DEBUG
	short	printed;
#endif
	struct	text	*next;
};

/*
 *	Text Block structure
 */
struct	textBlock {
#define	TB_SECNUM	1	/* section number	*/
#define	TB_PLAIN	2	/* plain text		*/
#define	TB_QUOTE	3	/* quoted text (>> etc)	*/
#define	TB_EXAMPLE	4	/* Example (verbatim)	*/
#define	TB_TABLE	5	/* table		*/
#define	TB_LIST		6	/* list			*/
#define	TB_LISTHD	7	/*  list head		*/
#define	TB_PAGE		8	/* new page		*/
#define	TB_SPACE	9	/* open region		*/
#define	TB_PICTURE	10	/* picture		*/
#define	TB_RIGHT	11	/* output device specific*/
#define	TB_CENTER	12	/* output device specific*/
#define	TB_RAW		13	/* output device specific*/
#define	TB_FTITLE	14	/* figure/table titles	*/
#define	TB_APPENDIX	15	/* appendix		*/
#define	TB_CAPSULE	16	/* encapsulation	*/
#define	TB_COMMENT	17	/* to be ignored	*/
#define TB_MARKUP	18	/* mark up */ /* KK (KOBAYASHI Kenichi) */
	short	type;		/* Type of block	*/
	short	rbegin;		/* Region begin		*/
	short	rend;		/* Region end		*/
	short	hinted;		/* hint exists		*/
	/* Pointer to ListHead (for LIST,LISTHD)*/
	struct textBlock *nextBlock;
	struct textBlock *superBlock;
};
struct	strVal {
	char	*pattern;
	short	value;
};
/*
 * Character translation table (for textQuote)
 */
struct transTable {
	char	special_char;
	char	*trans_to;
};

#define	MSG0(format)\
	if (verbose) (void)fprintf(stderr, format)
#define	MSG1(format, val)\
	if (verbose) (void)fprintf(stderr, format, val)
#define	MSG2(format, val1, val2)\
	if (verbose) (void)fprintf(stderr, format, val1, val2)
#define	MSG3(format, val1, val2, val3)\
	if (verbose) (void)fprintf(stderr, format, val1, val2, val3)
#define	MSG4(format, val1, val2, val3, val4)\
	if (verbose) (void)fprintf(stderr, format, val1, val2, val3,val4)
#ifdef	DEBUG
#define	DBG0(level, format)\
	if (debug >= level) (void)fprintf(stderr, format)
#define	DBG1(level, format, val)\
	if (debug >= level) (void)fprintf(stderr, format, val)
#define	DBG2(level, format, val1, val2)\
	if (debug >= level) (void)fprintf(stderr, format, val1, val2)
#define	DBG3(level, format, val1, val2, val3)\
	if (debug >= level) (void)fprintf(stderr, format, val1, val2, val3)
#define	DBG4(level, format, val1, val2, val3, val4)\
	if (debug >= level) (void)fprintf(stderr, format, val1, val2, val3,val4)
#define	PRINTED(lnum)	{if (texts[lnum]->printed++)\
fprintf(stderr, "ERROR: duplicated output %d:%s\n",lnum, texts[lnum]->body);}
#define	PRINTED2(l1, l2) {int i; for (i=l1; i<l2; i++)PRINTED(i);}
#else
#define	DBG0(level, format)
#define	DBG1(level, format, val)
#define	DBG2(level, format, val1, val2)
#define	DBG3(level, format, val1, val2, val3)
#define	DBG4(level, format, val1, val2, val3, val4)
#define	PRINTED(lnum)
#define	PRINTED2(l1, l2)
#endif

#define	abs(x)	((x)>0?(x):-(x))
#define	sign(x)	((x)>0?1:-1)
#define	max(a,b)	(a>b?a:b)
#define	min(a,b)	(a<b?a:b)
#define	alphaVal(c)	((c) - 'a' + 1)
#define	isBlank(l)	(texts[l]->length == 0 || texts[l]->block != NULL)
#define	str2match(s1, s2) ((*(s1) == *(s2)) && (*((s1) + 1) == *((s2) + 1)))

/*
 * Parameters for Parsing
 */
extern int tableEnabled;		/* Enables Table		*/
extern int jisTableEnabled;		/* JIS Wakusen Table enabled	*/
extern int indentedSecnum;		/* Section number maybe indented*/
extern int picEnabled;			/* Enables Picture		*/
extern int tableFactor;			/* Table judgement factor	*/
extern int examFactor;			/* Example judgement factor
					 * bigger number cause more example
					 */
extern int removePaging;		/* remove Paging (Header Footer lines)
					 * in the original text
					 */
extern int crossRefer;			/* Fig/Picture/Section reference*/
extern char *texStyleOpt;		/* TeX Style option		*/
#define	MM_MACRO	1
#define	MS_MACRO	2
extern int roffMacro;			/* roff macro (ms or mM)	*/
extern int renumber;			/* Only renumbering (section & list)*/
/*
 * Output parameters
 */
extern int pageWidth;			/* Page width (in point)	*/
extern int rawOutput;			/* text quote enabled(non through)*/
extern int halfCooked;			/* half cooked mode (not raw)	*/
#define	CODE_EUC	0
#define	CODE_JIS	1
#define	CODE_SJIS	2
extern int outputCode;			/* output code (JIS/EUC/SJIS)	*/
extern int inputCode;			/* input code (JIS,EUC/SJIS)	*/
extern int preamble;			/* preamble block		*/
extern int accurateSecnum;		/* Accurate section number output*/
extern int listDecor;			/* List decoration		*/
extern int reflectSpace;		/* reflect space lines		*/
extern int texQuote;			/* Use \verb| | for tex output	*/
#ifdef HTML
extern int htmlOnce;			/* One/Multi-path flag for HTML */
extern int htmlHere;			/* flag of pic/tbl here or not  */
extern int htmlOld;     /* KK */ /* old HTML mode for backward compatibility */
#endif
extern int indexEnabled;		/* Enables Indexing		*/
extern int fontSize;			/* font size			*/
extern int fontSpecified;		/* font size specified		*/
extern int useJverb;			/* use jverbatim (Nide)		*/
/*	*/
extern FILE *inFile;			/* intput File			*/
extern struct text	**texts;	/* texts[lineNum] array of text
					 * lineNum is 1 origin		*/
extern struct text	*textTop;	/* Top of text structure	*/
extern struct textBlock	*pageBp;	/* Top of new page block	*/
extern int textLines;			/* Text lines			*/
extern int textBegin;			/* Where text body begins	*/
extern int rightMargin;			/* Right margin of the text	*/
extern int japaneseText;		/* Japanese text		*/
extern char *fname;			/* Input file name		*/
extern int verbose;			/* verbose(generate warnings)	*/
extern int titleStyle;			/* TeX style			*/
extern int inlineHint;			/* Inline hint			*/
extern char *plain2Lib;			/* Plain2 library path		*/
extern int anySection;			/* if any section output exists */
#ifdef	DEBUG
extern int debug;			/* Debug  level			*/
#endif
/* 
 * Constants for title block
 */
#define	IND_CENTER	1	/* Centering		*/
#define	IND_RIGHT	2	/* Right justification	*/
#define	IND_RESUME	3	/* Resume from indentation */
#define	IND_INDENT	4	/* Resume from indentation */

#define	TITLE_A47	1
#define	TITLE_A47_1	2
#define	TITLE_OHP	3

/* Definition for begin/end	*/	
#define	BEGIN		1
#define	END		0

/* Definitions for spacing	*/
#define	SP_BLANK	1
#define	SP_PAGE		2
#define	SP_NEWLINE	3
#define	SP_SPACE	4

char	*codeCvt();
char	*sftj2eucStr();
char	*euc2sftjStr();
char	*euc2jisStr();
char	*strsave();
char	*strNum();
char 	*textQuote();
char 	*roffTextQuote();
struct	text	*prevLine();
struct	text	*nextLine();
struct	textBlock	*prevBlock();
struct	textBlock	*nextBlock();
struct	textBlock	*newTextBlock();
char	*listSecBody();
int	dumpText();
int	markIfList();
int	markIfPicture();
int	markIfExample();
int	markAsPlain();
int	markIfQuote();
int	markIfTable();
struct	picLineElm	*picLineMatch();
struct	picMiscLines	*picMiscMatch();

/*
 * Output driver
 */
struct	outDev {
	/*
	 * Text			(in {roff/tex}output.c)
	 */
	int (*plain)();		/*   plain text			*/
	int (*example)();	/*   example line		*/
	int (*listBlock)();	/* List Block (begin/end)	*/
	int (*dlistItem)();	/*   description list		*/
	int (*listItem)();	/*   enum/item  list		*/
	int (*rawText)();	/* Raw text output		*/
	int (*space)();		/* Blank lines			*/
	int (*capsule)();	/* Capsule block		*/
	int (*comment)();	/* Comment out			*/
#ifdef	PICTURE
	/*
	 * Picture
	 */
	int (*pictureBlock)();	/* Picture Block (begin/end)	*/
	int (*picLine)();	/*   line			*/
	int (*picArc)();	/*   arc			*/
	int (*picText)();	/*   text			*/
	int (*picLineWidth)();	/* Set line width		*/
#endif
	/*
	 * Table
	 */
	int (*table)();		/* Table			*/
	int (*fTitle)();	/* Figure/Table title		*/
	int (*title)();		/* Document title		*/
	/*
	 * Quoting
	 */
	char *(*quote1)();
	char *(*quote2)();
};
extern struct outDev	roffPut;
extern struct outDev	texPut;
#ifdef HTML
extern struct outDev	htmlPut;
#endif
extern struct outDev	*put;

#define	INLINE_BEGIN	"(("
#define	INLINE_END	"))"
#define INLINE_BEGIN_LEN	sizeof(INLINE_BEGIN)
#define INLINE_END_LEN		sizeof(INLINE_END)

#define	IL_PLAIN	0
#define	IL_RAW		1
#define	IL_FOOTNOTE	2
#define	IL_REFERENCE	3
#define	IL_BOLD		4
#define	IL_INDEX	5
#define IL_MARKUP	6	/* KK (KOBAYASHI Kenichi) */
#define IL_HYPERLINK	7	/* KK */
#define IL_SUBSCRIPT	8	/* KK */

extern	char	*malloc();

#define	INTERNAL_CODE	CODE_EUC

#ifdef	BSD
#include <strings.h>
#else
#include <string.h>
#define	index(s,c)	strchr(s,c)
#define bzero(buff, bytes)	memset(buff, 0, bytes)
#endif
#ifdef __MSDOS__		/* for Turbo C, Borland C */
#define MSDOS	1
#endif

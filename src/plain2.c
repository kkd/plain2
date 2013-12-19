/*
 * Copyright (C) 1991,1992,1994 NEC Corporation.
 */
#ifndef	lint
static char rcsid[] =
	"$Id: plain2.c,v 2.29 1994/04/19 10:40:29 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include <ctype.h>
#define	PLAIN2MAIN
#include "plain2.h"
#include "macro.h"

#define	PLAIN2_USAGE_1	"\
usage: plain2 [options] [files ...]\n\
  ---- parser options ----		   ---- output options ----(default)\n\
-table=dd: table factor	 [0-100](def=50) -roff:	    troff output\n\
-exam=dd:  example factor[0-100](def=50) -ms/-mm:   troff macro	       (mm)\n\
-indsec:   sections can be indented      -tex:      tex output\n\
-ktable:enable JIS keisen table		 -tstyle=ss:tex style\n\
-ref:	   figure/picture reference	 -renum:    renumbering only\n\
 					 -[no]listd:list decoration    (on)\n\
  ---- Others ----			 -[no]space:spacing            (on)\n\
-v:	   verbose output		 -[no]pre:  preamble block     (on)\n\
-dLevel:   debug level			 -[no]acursec: section numbers (off)\n\
  ----- experimental ----		 -raw:      quote special chars(off)\n\
-pt=Size:  font size			 -jis:      JIS code output\n\
 					 -sjis:     Shift-JIS code input/output\n\
 					 -f file:   output customization\n"
#define	VERSION	"plain2 r2.54 1994/04 by A.Uchida NEC Corporation"

extern	char	*getenv();
extern char	*optarg;
extern int	optind;
extern struct macDefs	roffMacros[], roffMsMacros[];
extern struct macDefs	texMacros[];
/*
 * Default parameters for Parsing
 */
int tableEnabled    =  1;
int jisTableEnabled =  0;
int indentedSecnum  =  0;
int tableFactor	    = 50;
int examFactor	    = 50;
int removePaging    =  0;
int crossRefer      =  0;
int inlineHint	    =  1;
int roffMacro	    = MM_MACRO;
int renumber	    =  0;
/*
 * Output parameters
 */
int pageWidth	    = DEF_PAGE_WIDTH;
int rawOutput	    =  0;
int halfCooked	    =  0;
int inputCode	    = CODE_EUC;
int outputCode	    = CODE_EUC;
int preamble	    =  1;
int accurateSecnum  =  0;
int listDecor	    =  1;
int reflectSpace    =  1;
int texQuote	    =  0;
int fontSize	    = 10;
int fontSpecified   =  0;
int verbose	    =  0;
char *plain2Lib	    = NULL;
char *macroName	    = NULL;
char *texStyle	    = NULL;
struct outDev *put  = &roffPut;

FILE	*inFile;
struct text	 **texts;
struct text	 *textTop;
struct textBlock *pageBp;
int	textLines;
int	textBegin;
int	rightMargin;
int	japaneseText;
int	titleStyle;
int	anySection = 0;
char	*fname;
int	acceptOutOption;
#ifdef	DEBUG
int	debug;
#endif

#ifdef	__TURBOC__
extern unsigned _stklen = 32768U;
#endif

main(argc, argv)
int	 argc;
char	 **argv;
{
	int	argind;
	/*
	 * Get options from Environment
	 *	or from the file specified by Env. variable
	 *
	 *  Parameters are set by ..
	 *	1) Default value
	 *	2) Environment (value itself or file)
	 *	3) Option line of each input files
	 */
	clearMacro();
	plain2Lib = getenv(PLAIN2_LIB);
	acceptOutOption = 1;
	envOption();
	if (getOption(argc, argv)) {
		(void)fprintf(stderr, PLAIN2_USAGE_1);
		(void)fprintf(stderr, "%s\n", VERSION);
		exit (2);
	}
	if (macroName)
		macroFile(macroName);
	saveParseDefaults();
	tblParseInit();
#ifdef	PICTURE
	picParseInit();
#endif
	ftitleInit();
	MSG1("%s\n", VERSION);
	argind = optind;		/* optind will be used afterward */
	if (argind >= argc) {
		fname = "stdin";
		inFile = stdin;
		doPlain2();
	}
	else while (argind < argc) {
		if ((inFile = fopen(argv[argind], "r")) == NULL) {
			(void)fprintf(stderr, "%s  cannot open\n",
				      argv[argind]);
			exit(1);
		}
		else {
			fname = strsave(argv[argind]);
		}
		doPlain2();
		acceptOutOption = 0;
		argind++;
	}
	if (anySection)
		putMacro(M_SECTION_END);
	if (preamble)
		putMacro(M_DOC_END);
	exit(0);
}
/*
 * Do parse & output
 */
doPlain2()
{
	static	int	firstTime = 1;

	restoreParseDefaults();
	readAndStoreFile();
	MSG2("Input %d lines from \"%s\"\n", textLines, fname);
	if (textLines <= 1)
		exit(0);
	analyzeLines(1, textLines);
	textBegin = getTitleBlock(1, textLines);

	if (firstTime) {
		firstTime = 0;
		if (put == &roffPut) {
			if (roffMacro == MS_MACRO)
				initMacroDefs(roffMsMacros);
			initMacroDefs(roffMacros);
		}
		else if (put == &texPut)
			initMacroDefs(texMacros);
		if (put == &texPut) {
			if (halfCooked) {
				texSetTrans(0);
			}
			else {
				texSetTrans(1);
			}
		}
		if (preamble) {
			if (texStyle)
				putMacro(M_DOC_BEGIN, (long)fontSize, texStyle);
			else
				putMacro(M_DOC_BEGIN, (long)fontSize, "");
		}
	}

	hint(textBegin, textLines);
	MSG0("[blank]");
	spacing(textBegin, textLines);
	if (removePaging) {
		MSG0("[paging]");
		headerFooter(textBegin, textLines);
	}
	MSG0("[section]");
	section(textBegin, textLines, 0);
	MSG0("[appendix]");
	appendix(textBegin, textLines);
	MSG0("[quotation]");
	quotedText(textBegin, textLines);
	MSG0("[list]");
	list(textBegin, textLines);
	MSG0("[fig title]");
	figTitle(textBegin, textLines);
#ifdef	PICTURE
	MSG0("[picture]");
	applyOnRegion2(textBegin, textLines, 0, markIfPicture);
	applyOnSpaced(textBegin, textLines, markIfPicture);
#endif
	if (tableEnabled) {
		MSG0("[table]");
		applyOnRegion(textBegin, textLines, markIfTable);
	}
	MSG0("[justified]");
	justified(textBegin, textLines);
	MSG0("[example]");
	applyOnRegion(textBegin, textLines, markIfExample);
	MSG0("[section]");
	section(textBegin, textLines, 1);
	MSG0("[plaintext]");
	plainText(textBegin, textLines);
	joinBlocks(textBegin, textLines, TB_EXAMPLE);
	joinBlocks(textBegin, textLines, TB_PICTURE);
	joinBlocks(textBegin, textLines, TB_CENTER);
	makeTitledBlock(textBegin, textLines);
	fixListRegion(textBegin, textLines);
	MSG0("\n");
	if (renumber)
		renumDocument();
	else
		outputDocument();
#ifdef	DEBUG
	{
		int	i;
		for (i = textBegin; i < textLines; i++)
			if (texts[i]->printed == 0)
				fprintf(stderr, "ERROR(Not Printed:%d)%s\n",
					i, texts[i]->body);
	}
#endif
	freeAll();
}
freeAll()
{
	int	l = 1;
	int	i;
	struct	textBlock	*tbp;
	while (l <= textLines) {
		if (tbp = texts[l]->block) {
			for (i = l; i < tbp->rend; i++) {
				free((char *)texts[i]->body);
				free((char *)texts[i]);
			}
			l = tbp->rend;
			free((char *)tbp);
		}
		else
			l++;
	}
	free((char *)texts);
}
#define	OUTPUT_OPTION(opt) {\
	if (!acceptOutOption) {\
		illegopt=opt;\
		goto dontOutOpt;\
	}\
 }\

getOption(xargc, xargv)
int	xargc;
char	**xargv;
{
	int	c;
	char	*illegopt;
	optind = 1;
	while ((c = getopt(xargc, xargv,
			   "X:j:l:s:vd:f:r:a:c:i:k:h:t:e:p:v:n:m:o:")) != -1)
		switch (c) {
		    case 'j':
#if	INTERNAL_CODE == CODE_EUC
			if (strcmp(optarg, "is") == 0) {
				OUTPUT_OPTION("-jis");
				outputCode = CODE_JIS;
			}
			else
#endif
				goto usage;
			break;
		    case 'l':
			if (strncmp(optarg, "istd", 4) == 0) {
				OUTPUT_OPTION("-listd");
				listDecor = 1;
			}
			break;
		    case 's':
#if	INTERNAL_CODE == CODE_EUC
			if (strcmp(optarg, "jis") == 0) {
				OUTPUT_OPTION("-sjis");
				outputCode = CODE_SJIS;
				inputCode  = CODE_SJIS;
			}
			else
#endif
				if (strcmp(optarg, "pace") == 0) {
					OUTPUT_OPTION("-space");
					reflectSpace = 1;
				}
				else goto usage;
			break;
		    case 'e':
#if	INTERNAL_CODE == CODE_EUC
			if (strcmp(optarg, "uc") == 0) {
				OUTPUT_OPTION("-euc");
				outputCode = CODE_EUC;
			}
			else
#endif
				if (strncmp(optarg, "xam=", 4) == 0) {
					examFactor = atoi(optarg+4);
					if (examFactor < 0 || examFactor >100){
						goto usage;
					}
				}
				else goto usage;
			break;
		    case 'v':
			OUTPUT_OPTION("-v");
			verbose = 1;
			break;
		    case 'p':
			if (strcmp(optarg, "re") == 0) {
				OUTPUT_OPTION("-pre");
				preamble = 1;
			}
			else if (strncmp(optarg, "t=", 2) == 0) {
				if ((fontSize = atoi(optarg + 2)) == 0)
					goto usage;
				fontSpecified = 1;
			}
			else goto usage;
			break;
		    case 'm':
			if (*optarg == 's') {
				OUTPUT_OPTION("-ms");
				roffMacro = MS_MACRO;
				put = &roffPut;
			}
			else if (*optarg == 'm') {
				OUTPUT_OPTION("-mm");
				roffMacro = MM_MACRO;
				put = &roffPut;
			}
			else	goto usage;
			break;
		    case 'r':
			if (strcmp(optarg, "aw") == 0) {
				OUTPUT_OPTION("-raw");
				rawOutput = 1;
			}
			else if (strcmp(optarg, "enum") == 0) {
				OUTPUT_OPTION("-renum");
				renumber = 1;
				preamble = 0;
			}
			else if (strcmp(optarg, "off") == 0) {
				OUTPUT_OPTION("-roff");
				put = &roffPut;
			}
			else if (strncmp(optarg, "mpage", 4) == 0)
				removePaging = 1;
			else if (strcmp(optarg, "ef") == 0) {
				crossRefer = 1;
			}
			else	goto usage;
			break;
		    case 'f':
			macroName = optarg;
			break;
		    case 'a':
			if (strcmp(optarg, "cursec") == 0) {
				OUTPUT_OPTION("-acursec");
				accurateSecnum = 1;
			}
			else	goto usage;
			break;
		    case 'i':
			if (strcmp(optarg, "ndsec") == 0) {
				indentedSecnum = 1;
			}
			else if (strcmp(optarg, "nline") == 0) {
				inlineHint = 1;
			}
			else	goto usage;
			break;
		    case 'k':
#ifdef	KANJI
			if (strncmp(optarg, "table", 5) == 0) {
				jisTableEnabled = 1;
			}
			else
#endif
				goto usage;
			break;
		    case 'h':
			if (strcmp(optarg, "alfraw") == 0) {
				OUTPUT_OPTION("-alfraw");
				halfCooked = 1;
			}
			else	goto usage;
			break;
		    case 't':
			if (strncmp(optarg, "able=", 5) == 0) {
				if ((tableFactor = atoi(optarg+5)) == 0)
					tableEnabled = 0;
				if (tableFactor < 0 || tableFactor > 100) {
					goto usage;
				}
			}
			else if (strcmp(optarg, "exq") == 0) {
				OUTPUT_OPTION("-texq");
				texQuote = 1;
				put = &texPut;
			}
			else if (strcmp(optarg, "ex") == 0) {
				OUTPUT_OPTION("-tex");
				put = &texPut;
			}
			else if (strncmp(optarg, "style=", 6) == 0) {
				OUTPUT_OPTION("-tstyle");
				texStyle = strsave(optarg + 6);
			}
			else	goto usage;
			break;
		    case 'n':
			if (strcmp(optarg, "opre") == 0) {
				OUTPUT_OPTION("-nopre");
				preamble = 0;
			}
			else if (strcmp(optarg, "ospace") == 0) {
				OUTPUT_OPTION("-nospace");
				reflectSpace = 0;
			}
			else if (strcmp(optarg, "oacursec") == 0) {
				OUTPUT_OPTION("-noacursec");
				accurateSecnum = 0;
			}
			else if (strcmp(optarg, "olistd") == 0) {
				OUTPUT_OPTION("-nolistd");
				listDecor = 0;
			}
			else if (strcmp(optarg, "oinline") == 0) {
				inlineHint = 0;
			}
			else goto usage;
			break;
#ifdef	DEBUG
		    case 'd':
			if ((debug = atoi(optarg)) == 0)
				goto usage;
			break;
#endif
		    case '?':
		    usage:
			return -1;
		    dontOutOpt:
			fprintf(stderr,
				"ERROR: Output option \"%s\" ignored.\n", illegopt);
			return -1;
		}
	return 0;
}
/*
 * Build "argc", "argv[]" from string and call getOption
 */
#define	MAX_ARGC	32
optionLine(s)
char	*s;
{
	static	int	xargc;
	static	char	*xargv[MAX_ARGC];
	int	inStr;
	inStr = 0;
	xargc = 1;
	while(*s) {
		if (!inStr && !isspace(*s)) {
			xargv[xargc] = s;
			xargc++;
			if (xargc >= MAX_ARGC) {
				(void)fprintf(stderr, "Too many options\n");
				return -1;
			}
			inStr = 1;
		}
		if (isspace(*s)) {
			*s = '\0';
			inStr = 0;
		}
		s++;
	}
	xargv[0] = "";
	if (getOption(xargc, xargv)
	    || optind < xargc)
		return -1;
	return 0;
}
/*
 * Get option from Environment variable
 */
envOption()
{
	char	*plainEnv;
	char	buf[MAX_LINE_LEN];
	FILE	*initf;
	if ((plainEnv = getenv(PLAIN2_ENV)) == NULL)
		return;
	if ((initf = fopen(plainEnv, "r")) == NULL) {
		char	envOpt[MAX_LINE_LEN];
		strcpy(envOpt, plainEnv);
		if (optionLine(envOpt)) {
			(void)fprintf(stderr,
				      "Illegal option in Environment %s\n",
				      PLAIN2_ENV);
			(void)fprintf(stderr,"%s\n", plainEnv);
			exit(1);
		}
	}
	else {
		int	line = 1;
		while (fgets(buf, MAX_LINE_LEN, initf) != NULL) {
			line++;
			if (optionLine(buf)) {
				(void)fprintf(stderr,
					      "illegal option in file %s at line %d\n",
					      plainEnv, line);
				(void)fprintf(stderr,"%s\n", buf);
				exit(1);
			}
		}
		fclose(initf);
	}
}
/*
 * Save & Restore Parameters for Parsing
 */
static int defJapaneseText;
static int defTableEnabled;
static int defJisTableEnabled;
static int defIndentedSecnum ;
static int defRawOutput;
static int defTableFactor;
static int defExamFactor;
/*
 * Parsing paramters
 *	1) Environment (from Env. variable itself  or from the specified file)
 *	2) Command argument option
 *
 * And "option" line of each files
 */
saveParseDefaults()
{
	defJapaneseText	   = japaneseText;
	defTableEnabled	   = tableEnabled;
	defJisTableEnabled = jisTableEnabled;
	defIndentedSecnum  = indentedSecnum;
	defRawOutput	   = rawOutput;
	defTableFactor	   = tableFactor;
	defExamFactor	   = examFactor;
}
restoreParseDefaults()
{
	japaneseText	= defJapaneseText;
	tableEnabled	= defTableEnabled;
	jisTableEnabled = defJisTableEnabled;
	indentedSecnum	= defIndentedSecnum;
	rawOutput	= defRawOutput;
	tableFactor	= defTableFactor;
	examFactor	= defExamFactor;
}


/*
 * Copyright (C) 1991,1992,1993 NEC Corporation.
 */
#ifndef lint
static char rcsid[]=
	"$Id: readfile.c,v 2.12 1994/04/19 10:17:00 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
#include "kanji.h"

char *
strsave(str)
char *str;
{
	char *save;
	save = (char *)malloc(strlen(str) + 1);
	if (save == NULL) {
		fprintf(stderr, "PANIC(malloc in strsave)\n");
		exit (2);
	}
	return strcpy(save, str);
}

/*
 * expand tab and Zenkaku space
 *	also convert JIS kanji code into EUC code
 */
#if	INTERNAL_CODE == CODE_EUC
void
expand(expanded, str)
register char	*expanded;
register char	*str;
{
	int	colPos = 0;
	int	secondByte = 0;
	int	jisInKanji = 0;

#ifdef	KANJI
	if (inputCode == CODE_SJIS) {
		static	char	cvtBuf[MAX_LINE_LEN];
		str = sftj2eucStr(str, cvtBuf);
	}
#endif
	for (; *str; str++) {
		/* for Jis Kanji Code		 */
		if (*str == ESC) {
			if (*(str+1) == '$'
			    && (*(str+2) == '@' || *(str+2) == 'B')) {
				jisInKanji = 1;
				str+=2;
				continue;
			}
			if (*(str+1) == '('
			    && (*(str+2) == 'J' || *(str+2) == 'B')) {
				jisInKanji = 0;
				str+=2;
				continue;
			}
		}
		if (jisInKanji && secondByte == 0){
			*str |= 0x80;
			*(str+1) |= 0x80;
		}
#ifdef	KANJI
		/* end for Jis Kanji Code	 */
		if (secondByte == 0
		    && isZenkakuSpc(str)) {
			/* Zenkaku Space	*/
			*expanded++ = ' ';
			*expanded++ = ' ';
			colPos+=2;
			str++;
		}
		else {
#endif
			if (*str == '\t')
				do {
					*expanded++ = ' ';
					colPos++;
				} while (colPos % 8 != 0);
			else {
				*expanded++ = *str;
				colPos++;
			}
			if (*str & 0x80) {
				secondByte = secondByte?0:1;
			}
#ifdef	KANJI
		}
#endif
	}
	if (secondByte) {
		/* input line split in 2-Byte Kanji char	*/
		(void)ungetc(*(expanded - 1), inFile);
		*(--expanded) = '\0';
	}
	else
		*expanded = '\0';
	while (*(--expanded) == ' ')
		*expanded = '\0';
}
#else
#if	INTERNAL_CODE == CODE_SJIS
void
expand(expanded, str)
register char	*expanded;
register char	*str;
{
    int	colPos = 0;
    for (; *str; str++) {
#ifdef	KANJI
	    if( isZenkakuSpc(str)) {
		    /* Zenkaku Space	*/
		    *expanded++ = ' ';
		    *expanded++ = ' ';
		    colPos+=2;
		    str++;
	    }
	    else if( isZenkaku(str)) {
		    *expanded++ = *str++;
		    *expanded++ = *str;
		    colPos+=2;
	    }
	    else {
#endif
		    if (*str == '\t')
			    do {
				    *expanded++ = ' ';
				    colPos++;
			    } while (colPos % 8 != 0);
		    else {
			    *expanded++ = *str;
			    colPos++;
		    }
#ifdef	KANJI
	    }
#endif
    }
    *expanded = '\0';
    while(
#ifdef	KANJI
	  !isZenkaku( expanded-sizeof(char)) &&
#endif
	  *(expanded) == ' ' )
	    *(expanded--) = '\0';
}
#else
unknown code;
#endif
#endif
storeLine(textp, str)
struct	text	*textp;
char	*str;
{
	static char	expanded[MAX_LINE_LEN];
	int	i, spaces, nonspace, indent;
	short	japCode;
	expand(expanded, str);
	textp->body   = str = strsave(expanded);
	indent   = 0;
	japCode  = 0;
	nonspace = 0;
	spaces   = 0;
	for (i=0; *str; str++, i++) {
		if (nonspace == 0 && *str != ' ') {
			indent   = i;
			nonspace =1;
		}
		if (*str & 0x80)
			japCode = 1;
		else if (nonspace && *str == ' ')
			spaces++;
	}		
	textp->japanese = japCode;
	textp->indent   = indent;
	japaneseText   |=japCode;
	if (nonspace) {
		textp->length = i;
		textp->spaces = spaces;
	}
	else {
		textp->blank  = 1;
		textp->length = 0;
		textp->spaces = 0;
	}
}
/*
 * Read and store file
 */
readAndStoreFile()
{
	struct	text	*textp;
	struct	text	Top;
	char	buf[MAX_LINE_LEN];
	int	i, len;
	japaneseText =   0;
	textp = &Top;
	textLines = 1;
	while (fgets(buf, MAX_LINE_LEN, inFile) != NULL) {
		textp->next   = (struct text *)malloc(sizeof(struct text));
		if (textp->next == NULL){
			fprintf(stderr, "PANIC(malloc in readAndStore)\n");
			exit (2);
		}
		textp  = textp->next;
		bzero((char *)textp, sizeof(struct text));
		textp->next   = NULL;

		if (buf[len=strlen(buf)-1] == '\n')
			buf[len] = '\0';
		storeLine(textp, buf);
		textLines++;
	}
	textTop  = Top.next;
	/*
	 * Build texts[] array for later access
	 *	texts[0] : not exist
	 *	texts[1] - texts[textLines - 1] : text lines
	 *	texts[textLines] : terminator (null)
	 */
	texts = (struct text **)
		malloc(sizeof(struct text *) * (textLines + 1));
	for (textp = textTop, i=1; i < textLines; textp = textp->next, i++)
		texts[i] = textp;
	texts[textLines] = (struct text *)malloc(sizeof(struct text));
	bzero((char *)texts[textLines], sizeof(struct text));
}

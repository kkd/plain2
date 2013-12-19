/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: asccvt.c,v 2.9 1994/04/19 10:16:19 uchida Exp $ (NEC)";
#endif
/*
 * Convert Kanji(EUC) string to Octal format ("\122\102...")
 */
#include <stdio.h>
main()
{
	char	c;
	while ((c = getchar()) != EOF)
		if (c & 0x80)
			printf("\\%o",(unsigned char)c);
		else
			putchar(c);
	exit(0);
}

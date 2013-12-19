/*
 * Copyright (C) 1991,1992 NEC Corporation.
 * $Id: kanji.h,v 2.10 1994/04/19 10:16:45 uchida Exp $ (NEC)
 */
#ifdef	KANJI
/* Internal Kanji Code
 *	内部コード．ソースプログラム自身も，ここで定義されたコードでなければ
 *	ならない．
 */
#if	INTERNAL_CODE == CODE_EUC
/*
 * 内部処理コードEUC用
 */
/* 全角である		*/
#define	isZenkaku(str)		((*(str) & 0x80) == 0x80)

/* 全角スペースである		*/
#define	isZenkakuSpc(str)	(*(str) == (char)0xa1 && *((str)+1) == (char)0xa1)

/* 全角数字らしい(第一バイトのみで判定)	*/
#define	maybeZenkakuNum(str) 	(*(str) == (char)0xa3)

/* 全角数字の値(０１…９) を求める	*/
#define	ZenkakuNumVal(str) 	(*((str)+1) & 0x0f)

/* 細い罫線素片の第一バイトに一致する	*/
#define	maybeThinKeisen(str) 	(*(str) == (char)0xa8)

#else
#if	INTERNAL_CODE == CODE_SJIS
/*
 *	内部処理コードshift JIS用
 */
typedef unsigned char	UCHAR;

/* 全角である		*/
#define	isZenkaku(str) (((UCHAR)0x80 <= (UCHAR)(*(str))&&(UCHAR)(*(str)) <= (UCHAR)0x9f)|| \
			((UCHAR)0xe0 <= (UCHAR)(*(str))&&(UCHAR)(*(str)) <= (UCHAR)0xff))

/* 全角スペースである		*/
#define	isZenkakuSpc(str) ((UCHAR)*(str) == (UCHAR)0x81 && (UCHAR)*((str)+1) == (UCHAR)0x40)

/* 全角数字 */
#define	maybeZenkakuNum(str) (((UCHAR)*(str) == (UCHAR)0x82) && \
		   ((UCHAR)0x4f <= (UCHAR)(*(str)+1)||(UCHAR)(*(str)+1) <= (UCHAR)0x58) )

/* 全角数字の値(０１…９) を求める	*/
#define	ZenkakuNumVal(str) 	((UCHAR)*((str)+1) - (UCHAR)0x4f)

/* 細い罫線素片の第一バイトに一致する	*/
#define	maybeThinKeisen(str) 	( ((UCHAR)*(str) == (UCHAR)0x84) && \
                 ((UCHAR)0x9f <= (UCHAR)(*(str)+1) || (UCHAR)(*(str)+1) <= (UCHAR)0xa9))

#else
unknown code;
#endif
#endif
#endif

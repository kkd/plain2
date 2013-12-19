/*
 * Copyright (C) 1991,1992 NEC Corporation.
 * $Id: kanji.h,v 2.10 1994/04/19 10:16:45 uchida Exp $ (NEC)
 */
#ifdef	KANJI
/* Internal Kanji Code
 *	���������ɡ��������ץ���༫�Ȥ⡤������������줿�����ɤǤʤ����
 *	�ʤ�ʤ���
 */
#if	INTERNAL_CODE == CODE_EUC
/*
 * ��������������EUC��
 */
/* ���ѤǤ���		*/
#define	isZenkaku(str)		((*(str) & 0x80) == 0x80)

/* ���ѥ��ڡ����Ǥ���		*/
#define	isZenkakuSpc(str)	(*(str) == (char)0xa1 && *((str)+1) == (char)0xa1)

/* ���ѿ����餷��(���Х��ȤΤߤ�Ƚ��)	*/
#define	maybeZenkakuNum(str) 	(*(str) == (char)0xa3)

/* ���ѿ�������(�����ģ�) �����	*/
#define	ZenkakuNumVal(str) 	(*((str)+1) & 0x0f)

/* �٤��������Ҥ����Х��Ȥ˰��פ���	*/
#define	maybeThinKeisen(str) 	(*(str) == (char)0xa8)

#else
#if	INTERNAL_CODE == CODE_SJIS
/*
 *	��������������shift JIS��
 */
typedef unsigned char	UCHAR;

/* ���ѤǤ���		*/
#define	isZenkaku(str) (((UCHAR)0x80 <= (UCHAR)(*(str))&&(UCHAR)(*(str)) <= (UCHAR)0x9f)|| \
			((UCHAR)0xe0 <= (UCHAR)(*(str))&&(UCHAR)(*(str)) <= (UCHAR)0xff))

/* ���ѥ��ڡ����Ǥ���		*/
#define	isZenkakuSpc(str) ((UCHAR)*(str) == (UCHAR)0x81 && (UCHAR)*((str)+1) == (UCHAR)0x40)

/* ���ѿ��� */
#define	maybeZenkakuNum(str) (((UCHAR)*(str) == (UCHAR)0x82) && \
		   ((UCHAR)0x4f <= (UCHAR)(*(str)+1)||(UCHAR)(*(str)+1) <= (UCHAR)0x58) )

/* ���ѿ�������(�����ģ�) �����	*/
#define	ZenkakuNumVal(str) 	((UCHAR)*((str)+1) - (UCHAR)0x4f)

/* �٤��������Ҥ����Х��Ȥ˰��פ���	*/
#define	maybeThinKeisen(str) 	( ((UCHAR)*(str) == (UCHAR)0x84) && \
                 ((UCHAR)0x9f <= (UCHAR)(*(str)+1) || (UCHAR)(*(str)+1) <= (UCHAR)0xa9))

#else
unknown code;
#endif
#endif
#endif

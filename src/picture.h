/*
 * Copyright (C) 1991,1992 NEC Corporation.
 * $Id: picture.h,v 2.11 1994/04/19 10:16:54 uchida Exp $ (NEC)
 */
#ifdef	PICTURE
/*
 * ����β��Ϥ�ɽ���Τ�������
 */
extern int	picFontSize;		/* �����ΰ�ǻȤ��ե���ȥ�����	*/
/*
 * ��������ʸ���򵭽Ҥ��뤿������
 */
struct	stroke {
#define	LINE_NULL	0	/* ���Ϥʤ�		*/

#define	VECT_BGN	1	/* ���ޤ��Ͼ���������		*/
#define	VECT_END	2	/* ���ޤ��ϲ����������		*/
#define	VECT_BOTH	3	/* ξ�������			*/

#define	LINE_FULL	4	/* ��������			*/
#define	LINE_SEP	5	/* ������������˸���������	*/
#define	LINE_BGN	6	/* ��Ⱦʬ�ޤ��ϲ�Ⱦʬ����������	*/
#define	LINE_END	7	/* ��Ⱦʬ�ޤ��Ͼ�Ⱦʬ����������	*/
	short	type;
#define	SY_NULL		0	/* ���Ϥʤ�	*/
#define	SY_THIN		1	/* �٤���	*/
#define	SY_THICK	2	/* ������	*/
#define	SY_DASH		3	/* ����		*/
#define	SY_DBL		4	/* 2��������	*/
#define	SY_ELPS		5	/* �ʱ�		*/
#define	SY_HELPS	6	/* ����Ⱦʬ�ʱ�	*/
#define	SY_CIRCLE	7	/* ��..?	*/
	short	style;
};
enum	direction {dir_br, dir_bl, dir_tl, dir_tr};
#endif

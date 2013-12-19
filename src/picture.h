/*
 * Copyright (C) 1991,1992 NEC Corporation.
 * $Id: picture.h,v 2.11 1994/04/19 10:16:54 uchida Exp $ (NEC)
 */
#ifdef	PICTURE
/*
 * 線画の解析と表示のための定義
 */
extern int	picFontSize;		/* 線画領域で使うフォントサイズ	*/
/*
 * 罫線素片文字を記述するための定義
 */
struct	stroke {
#define	LINE_NULL	0	/* 線はない		*/

#define	VECT_BGN	1	/* 左または上向きの矢印		*/
#define	VECT_END	2	/* 右または下向きの矢印		*/
#define	VECT_BOTH	3	/* 両方に矢印			*/

#define	LINE_FULL	4	/* 完全な線			*/
#define	LINE_SEP	5	/* 細線だが中央に交点がある	*/
#define	LINE_BGN	6	/* 右半分または下半分に線がある	*/
#define	LINE_END	7	/* 左半分または上半分に線がある	*/
	short	type;
#define	SY_NULL		0	/* 線はない	*/
#define	SY_THIN		1	/* 細い線	*/
#define	SY_THICK	2	/* 太い線	*/
#define	SY_DASH		3	/* 点線		*/
#define	SY_DBL		4	/* 2倍太い線	*/
#define	SY_ELPS		5	/* 楕円		*/
#define	SY_HELPS	6	/* した半分楕円	*/
#define	SY_CIRCLE	7	/* 円..?	*/
	short	style;
};
enum	direction {dir_br, dir_bl, dir_tl, dir_tr};
#endif

/*
 * Copyright (C) 1991,1992 NEC Corporation.
 * $Id: table.h,v 2.9 1994/04/19 10:17:07 uchida Exp $ (NEC)
 */
#define	TBL_MIN_LINES	2	/* Minimum number of lines to be table	*/
#define	TBL_MIN_FIELDS	2	/* Minimum number of fields to be table	*/
#define TBL_MAX_FIELDS	32	/* Maximum number of fields		*/

#define	HORI_DBL_ALL	1		/* ====================	*/
#define	HORI_SNGL_ALL	2		/* --------------------	*/
#define	HORI_EXIST	3		/* |          |-------| */
#define	HORI_NULL	0		/* No horizontal lines	*/

/*		|left	 |right	
 *		v	 v
 *      	field-1  ||	<-vlines == 2
 */
struct	tblField {
	int	left;		/* Left of the field	*/
	int	right;		/* Right of the field	*/
	int	align;
	int	vlines;		/* Number of vertical lines follow	*/
	char	defFormat;	/* Default format of the field		*/
};
/*
 * Table structure
 *	t_field[0]: left boundary
 *	t_field[1]-[t_nfield - 1]: table body
 */
struct	table {
	struct	tblField	t_field[TBL_MAX_FIELDS];
	int			t_nfield;
};
#define	tbl_field	(tblp->t_field)
#define	tbl_nfield	(tblp->t_nfield)

char	*tblSubstr();

/*
 * Copyright (C) 1991,1992 NEC Corporation.
 */
#ifndef lint
static char rcsid[] =
	"$Id: title.c,v 2.15 1994/04/19 10:17:10 uchida Exp $ (NEC)";
#endif

#include <stdio.h>
#include "plain2.h"
/*
 * 各入力の先頭にあるタイトル部の解析
 */
#define	MAX_TITLE_LEN	64	/* タイトルブロックの最大行数	*/
#define	MAX_ENTRY_LEN	16	/* 各エントリの最大行数		*/
/*
 * タイトルブロックのエントリ種別
 */
#define TENT_TITLE		1
#define TENT_NUMBER		2
#define TENT_DATE		3
#define TENT_SHOZOKU		4
#define TENT_HAIFU		5
#define TENT_NAME		6
#define TENT_RENRAKU		7

#ifdef	TITLE_SPECIAL
#define	TENT_SPECIAL1		1
#define	TENT_SPECIAL2		2
#define	TENT_SPECIAL3		3
#define	TENT_SPECIAL4		4
#define	TENT_SPECIAL5		5
#define	TENT_SPECIAL6		6
#define	TENT_SPECIAL7		7
#define	TENT_SPECIAL8		8
#define	TENT_SPECIAL9		9
#define	TENT_MAX		9	/* 通常のエントリはここまで	*/
#else
#define	TENT_MAX		7	/* 通常のエントリはここまで	*/
#endif

/* コントロールのためのエントリ	*/
#define TENT_STYLE		(TENT_MAX + 1)
#define TENT_OPTION		(TENT_MAX + 2)
#define TENT_COMMENT		(TENT_MAX + 3)

char *titleElements[TENT_MAX+1][MAX_ENTRY_LEN];
struct	titleText{
	char	*body;
	int	kind;
	int	outlooks;
} htext[MAX_TITLE_LEN];
struct	titleKey {
	char	*pattern;
	short	kind;
#define	S_ALL		1
#define	S_FROMRIGHT	2
#define	S_FROMLEFT	3
#define	S_CONTROL	4
	short	outlooks;
	short	value;
#define	W_ANYWHERE	1
#define	W_BEGIN		2
	short	where;
} titleKeyTbl[] = {
#ifdef	NEC_CCS
	/*
	 * NEC C&C Common software dependent
	 */
	"a471",		TENT_STYLE,	S_CONTROL,	TITLE_A47_1, W_BEGIN,
	"A471",		TENT_STYLE,	S_CONTROL,	TITLE_A47_1, W_BEGIN,
	"shounin",	TENT_STYLE,	S_CONTROL,	TITLE_A47_1, W_BEGIN,
	"a47",		TENT_STYLE,	S_CONTROL,	TITLE_A47,   W_BEGIN,
	"A47",		TENT_STYLE,	S_CONTROL,	TITLE_A47,   W_BEGIN,
#endif
#ifdef	TITLE_SPECIAL
	"引数1",	TENT_SPECIAL1,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数2",	TENT_SPECIAL2,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数3",	TENT_SPECIAL3,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数4",	TENT_SPECIAL4,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数5",	TENT_SPECIAL5,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数6",	TENT_SPECIAL6,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数7",	TENT_SPECIAL7,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数8",	TENT_SPECIAL8,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"引数9",	TENT_SPECIAL9,	S_FROMRIGHT,	0,	W_ANYWHERE,
#endif
	"comment",	TENT_COMMENT,	S_CONTROL,	0,	W_BEGIN,
	"option",	TENT_OPTION,	S_CONTROL,	0,	W_BEGIN,
	"題名",		TENT_TITLE,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"『",		TENT_TITLE,	S_ALL,		0,	W_ANYWHERE,
	"【",		TENT_TITLE,	S_ALL,		0,	W_ANYWHERE,
	"配付先",	TENT_HAIFU,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"配布先",	TENT_HAIFU,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"各位",		TENT_HAIFU,	S_ALL,		0,	W_ANYWHERE,
	"担当",		TENT_NAME,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"著者",		TENT_NAME,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"by",		TENT_NAME,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"所属",		TENT_SHOZOKU,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"本部",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"開発部",	TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"事業部",	TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"研究所",	TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"研究部",	TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
/* Add Nide */
	"大学",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"学部",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"学科",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"研究室",	TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
/* Add Nide end */
	"文書番号",	TENT_NUMBER,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"日付",		TENT_DATE,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"月",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"日",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"連絡先",	TENT_RENRAKU,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"contact",	TENT_RENRAKU,	S_FROMRIGHT,	0,	W_ANYWHERE,
	"殿",		TENT_HAIFU,	S_ALL,		0,	W_ANYWHERE,
	"様",		TENT_HAIFU,	S_ALL,		0,	W_ANYWHERE,
	"TEL",		TENT_RENRAKU,	S_ALL,		0,	W_ANYWHERE,
	"tel",		TENT_RENRAKU,	S_ALL,		0,	W_ANYWHERE,
	"Tel",		TENT_RENRAKU,	S_ALL,		0,	W_ANYWHERE,
	"90/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"91/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"92/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"93/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"94/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"95/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"96/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"97/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"98/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"99/",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/90",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/91",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/92",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/93",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/94",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/95",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/96",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/97",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/98",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/99",		TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
/* Add Nide */
	"2000/",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"2001/",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"2002/",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"2003/",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"2004/",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/2000",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/2001",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/2002",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/2003",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
	"/2004",	TENT_DATE,	S_ALL,		0,	W_ANYWHERE,
/* Add Nide end */
#ifdef	NEC_CCS
	"開本",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"開研",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"技研",		TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"企画室",	TENT_SHOZOKU,	S_ALL,		0,	W_ANYWHERE,
	"技E",		TENT_NUMBER,	S_ALL,		0,	W_ANYWHERE,
	"技M",		TENT_NUMBER,	S_ALL,		0,	W_ANYWHERE,
#endif
};
/*
 * 文字列がどれかのエントリにマッチするか調べる
 */
keyWordMatch(str, kind, outlooks, val)
char	*str;
int	*kind;
int	*outlooks;
int	*val;
{
	int	i;
	char	*s;
	*kind = *outlooks = *val = 0;
	for (i=0; i<sizeof(titleKeyTbl)/sizeof(struct titleKey); i++) {
		for (s = str; *s != '\0'; s++){
			if (strncmp(titleKeyTbl[i].pattern, s,
				    strlen(titleKeyTbl[i].pattern)) == 0) {
				DBG2(8, "title Matched <%s>%s\n",
					titleKeyTbl[i].pattern, s);
				*kind     = titleKeyTbl[i].kind;
				*outlooks = titleKeyTbl[i].outlooks;
				*val      = titleKeyTbl[i].value;
				return;
			}
			if (titleKeyTbl[i].where == W_BEGIN)
				/* このパターンは行の先頭から始まらなければ
				 * ならない
				 */
				break;
		}
	}
}
/*
 * タイトルブロックの有無を調べる
 * タイトルブロックがあった場合，その末尾を返す．なかった場合は
 * そのまま先頭を返す
 */
getTitleBlock(begin, end)
int	begin;
int	end;
{
	int	l, val;
	/* ブロック内で最初の空行を探す		*/
	for (l = begin; l < end; l++)
		if (texts[l]->length == 0)
			break;
	end = l;
	if (end - begin > MAX_TITLE_LEN)
		/* ブロック行数が長過ぎるので除外する	*/
		return begin;
	for (l = begin; l < end; l++) {
		keyWordMatch(texts[l]->body + texts[l]->indent,
			     &(htext[l].kind), &(htext[l].outlooks),
			     &val);
		DBG3(3,"TITLE %d:%d = %d\n", htext[l].kind,
			htext[l].outlooks, val);
		if (htext[l].kind ==  TENT_STYLE){
			DBG1(3,"TITLE Control = %d\n", val);
			titleStyle = val;
		}
		else if (htext[l].kind ==  TENT_OPTION){
			char	*s;
			s = texts[l]->body + texts[l]->indent;
			DBG1(3,"TITLE Option = %s\n", s);
			while (*s != '\0' && *s != ' ')
				s++;
			if (optionLine(s)) {
				(void)fprintf(stderr,
					      "Illegal option in %s\n%s\n",
					      fname, s);
				exit(1);
			}
		}
		if (htext[l].kind != 0)
			switch (htext[l].outlooks) {
			    case S_ALL:
				/* 行全体をとる	*/
				htext[l].body =
					texts[l]->body + texts[l]->indent;
				DBG2(6,"Title Hit %d %s\n", htext[l].kind,
					htext[l].body);
				break;
			    case S_FROMRIGHT:
				/* 右端から最初の空白までをとる	*/
				{
					char	*s, *single_sp;
					s =  texts[l]->body + texts[l]->length
						- 1;
					single_sp = s;
					while (s > texts[l]->body + texts[l]->indent) {
						if (*s != ' ')
							break;
						s--;
					}
					while (s > texts[l]->body + texts[l]->indent) {
						if (*s == ' ')
							single_sp = s;
						if (*s == ' '
						    && *(s - 1) == ' ')
							break;
						s--;
					}
					if (s == texts[l]->body + texts[l]->indent)
						s = single_sp;
					htext[l].body = ++s;
					DBG2(6, "Title Hit %d %s\n", htext[l].kind,
						htext[l].body);
					break;
				}
			    case S_FROMLEFT:
				/* 左端から最初の空白までをとる	*/
				fprintf(stderr, "PANIC(unknown title entry)\n");
				exit(2);
			    default:
				break;
			}
	}
	saveTitleBody(begin, end, TENT_HAIFU);
	saveTitleBody(begin, end, TENT_TITLE);
	saveTitleBody(begin, end, TENT_SHOZOKU);
	saveTitleBody(begin, end, TENT_NUMBER);
	saveTitleBody(begin, end, TENT_DATE);
	saveTitleBody(begin, end, TENT_NAME);
	saveTitleBody(begin, end, TENT_RENRAKU);
#ifdef	TITLE_SPECIAL
	saveTitleBody(begin, end, TENT_SPECIAL8);
	saveTitleBody(begin, end, TENT_SPECIAL9);
#endif
	for (l = begin; l < end; l++)
		if (htext[l].kind == 0) {
			titleStyle = 0;
			return begin;
		}
	return end;
}
/*
 * タイトルから両端の括弧を取り除く
 */
char	*
deleteParen(str)
char	*str;
{
	char	*s;
	if (strncmp(str,"【", 2) == 0) {
		for (s=str; *s!= '\0'; s++)
			if (strncmp(s,"】", 2) == 0) {
				*s = '\0';
				return str+2;
			}
	}
	if (strncmp(str,"『", 2) == 0) {
		for (s=str; *s!= '\0'; s++)
			if (strncmp(s,"』", 2) == 0) {
				*s = '\0';
				return str+2;
			}
	}
	return str;
}
/*
 * タイトルブロックをエントリごとのエリアに保存する
 */
saveTitleBody(begin, end, kind)
int	kind;
int	begin;
int	end;
{
	int	l, n;
	n = 0;
	for (l = begin; l < end; l++) {
		if (htext[l].kind == kind) {
			titleElements[kind][n] = strsave(htext[l].body);
			if (kind == TENT_TITLE) {
				titleElements[kind][n]
					= deleteParen(titleElements[kind][n]);

			}
			if (n++ >= MAX_ENTRY_LEN)
				break;
		}
	}
	titleElements[kind][n] = NULL;
}
outputTitle()
{
	(*put->title)(titleStyle,
		      titleElements[TENT_HAIFU], titleElements[TENT_TITLE],
		      titleElements[TENT_SHOZOKU],titleElements[TENT_NUMBER],
		      titleElements[TENT_DATE],  titleElements[TENT_NAME],
		      titleElements[TENT_RENRAKU]
#ifdef	TITLE_SPECIAL
		      ,titleElements[TENT_SPECIAL8],
		      titleElements[TENT_SPECIAL9]
#endif
		      );
}

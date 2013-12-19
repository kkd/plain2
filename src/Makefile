# Copyright (C) 1991,1992,1993 NEC Corporation.
#
# $Id: Makefile,v 2.20 1994/04/19 10:35:38 uchida Exp $ (NEC)
#
#	Makefile for plain2 1993 by A.Uchida 
#
#
BINDIR=/usr/local/bin
LIBDIR=/usr/local/lib/plain2

SHELL=/bin/sh

DEFINES=-DKANJI -DPICTURE

#CC=gcc
#CFLAGS= -fwritable-strings ${DEFINES} -DBSD -DDEBUG

CC=cc
CFLAGS= -O ${DEFINES} -DBSD -DDEBUG
#CFLAGS=  -g ${DEFINES} -DDEBUG

SRCS=plain2.c readfile.c analyze.c miscutil.c\
	section.c example.c list.c plaintxt.c table.c spacing.c\
	headfoot.c hint.c picture.c justify.c ftitle.c\
	roffout.c texout.c title.c output.c renumout.c macro.c
OBJS_NOKANJI=plain2.o readfile.o miscutil.o\
	section.o example.o list.o plaintxt.o spacing.o\
	headfoot.o hint.o justify.o\
	texout.o output.o renumout.o macro.o
#
# Following files contain Kanji characters(8bit) in EUC code.
#
OBJS_KANJI=roffout.o analyze.o table.o title.o picture.o ftitle.o

HDRS=plain2.h table.h picture.h kanji.h
LIBS=
LDFLAGS=

plain2: ${OBJS_NOKANJI} ${OBJS_KANJI}
	${CC} ${CFLAGS} ${LDFLAGS} -o plain2 ${OBJS_NOKANJI} ${OBJS_KANJI} ${LIBES}

lint:
	lint ${DEFINES} ${SRCS} > error.LINT 2>&1 

TAGS: ${SRCS}
	etags ${SRCS}

install:
	install -s plain2 ${BINDIR}
	cp ../lib/*.p2 ${LIBDIR}

clean:
	rm -f plain2 *.o asccvt x.c error.LINT

wc:
	@cstep ${HDRS} ${SRCS}

depend:
	makemake -f Makefile  ${HDRS} ${SRCS}

asccvt: asccvt.c
	cc -o asccvt asccvt.c

${OBJS_NOKANJI}:

${OBJS_KANJI}:

#${OBJS_KANJI}: asccvt
#	asccvt < $< > x.c
#	cc -c ${CFLAGS} x.c
#	@rm -f x.c
#	mv x.o $@

.SUFFIXES: .c .o .h

# DO NOT DELETE THIS LINE -- makemake depend uses it

plain2.o: plain2.h macro.h
readfile.o: plain2.h kanji.h
analyze.o: plain2.h picture.h kanji.h
miscutil.o: plain2.h kanji.h
section.o: plain2.h macro.h
example.o: plain2.h
list.o: plain2.h
plaintxt.o: plain2.h
table.o: plain2.h table.h kanji.h
spacing.o: plain2.h
headfoot.o: plain2.h
hint.o: plain2.h
picture.o: plain2.h kanji.h picture.h
justify.o: plain2.h
ftitle.o: plain2.h
roffout.o: plain2.h picture.h table.h macro.h
texout.o: plain2.h picture.h table.h macro.h
title.o: plain2.h
output.o: plain2.h picture.h macro.h
renumout.o: plain2.h
macro.o: plain2.h macro.h

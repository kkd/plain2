#
# Copyright (C) 1993 A.Uchida uchida@ccs.mt.nec.co.jp
#
# $Id: Makefile,v 1.3 1994/04/19 09:32:40 uchida Exp $
#
SAMPLES=legend.raw legend.tex beginning.raw title_pat1.raw title_pat2.raw\
        section_renum.raw section_renum.tex paragraph.raw paragraph.tex\
        list_item.raw list_item.tex list_enum.raw list_enum.tex\
	list_desc.raw list_desc.tex table_intro.raw\
        table_intro.tex picture_intro.raw picture_intro.tex\
        option_title.raw option_title.tex hint_fail.raw hint_fail.tex\
        hint_ok.raw hint_ok.tex hint_all.raw hint_none.raw\
	hint_raw.raw hint_raw.texv\
	inline.raw\
        inline.tex title_pat1.raw title_pat2.raw title_pat3.raw\
        title_pat1.tex title_pat4.raw title_pat4.tex title_fail.raw\
        title_fail.tex title_cmplx.raw title_cmplx.tex section_intro.raw\
        section_intro.tex section_list.raw section_list.tex\
        section_ind.raw section_ind.tex section_renum2.raw\
        section_renum2.tex section_accur.raw section_accur.tex\
        section_deep.raw section_deep.tex list_not.raw list_not.tex\
        list_bullet.raw list_bullet.tex list_enumbad.raw list_enumbad.tex\
        list_renum.raw list_renum.tex list_nest.raw list_nest.tex\
        list_enumcmp.raw list_enumcmp.lofi list_enumcmp.tex list_dscmp.raw\
        list_dscmp.tex list_dscspl.raw list_dscspl.tex list_cont.raw\
        list_cont.tex table_smpl.raw table_smpl.tex table_adjust.raw\
        table_adjust.tex table_adjust2.texv table_cmplx.raw\
        table_cmplx.tex table_kanji.raw table_kanji.tex table_align.raw\
        table_align.tex table_fail.raw table_fail.tex picture_line.raw\
        picture_line.tex picture_slant.raw picture_slant.tex\
        picture_round.raw picture_round.tex picture_disk.raw\
        picture_disk.tex picture_align.raw picture_align.tex\
        picture_adjust.raw picture_adjust.tex example_intro.raw\
        example_adjust.raw example_adjust.tex \
        space_intro.raw space_intro.texv indentation.raw indentation.tex \
	alignment.raw\
        alignment.tex appendix.raw renumber.raw renumber_out.raw\
        section_star.raw section_star.texv section_stars.tex reference.raw\
        tgif_doc.raw title_a47.raw title_a47.tex title_shounin.raw\
        title_shounin.tex header_text.raw ftitle_illeg.raw ftitle_illeg.tex\
	help_message.raw

OTHERFILES=tgif_fig.eps tgif_win.eps logo.eps modules.eps help_message

RAWTEXFILES=Tgif_doc.tex Beginning.tex Hint_all.tex \
	Example_adjust2.tex

PRTGIF=prtgif+
PLAIN2=../src/plain2
VERBTEXFILES=

plain2.ps: plain2.dvi
	dvi2ps -p400 plain2.dvi > $@

plain2.dvi: ${SAMPLES} ${RAWTEXFILES} ${OTHERFILES} plain2.tex
	jlatex plain2.tex
	jlatex plain2.tex

plain2.tex: plain2.pln guide.p2
	$(PLAIN2) -tex -ref -fguide plain2.pln > plain2.tex

clean:
	rm -f  ${SAMPLES} ${OTHERFILES} plain2.tex plain2.dvi plain2.ps \
	plain2.toc plain2.aux plain2.log

title_a47.tex title_shounin.tex:
	$(PLAIN2) -tex -fexampl_left  $< > $@

table_kanji.tex: table_kanji.pln
	$(PLAIN2) -tex -ktable -fexampl $< > $@

section_ind.tex: section_ind.pln
	$(PLAIN2) -tex -indsec -fexampl $< > $@

section_accur.tex: section_accur.pln
	$(PLAIN2) -tex -acursec -fexampl $< > $@

picture_align.tex: picture_align.pln
	$(PLAIN2) -tex -fexampl $< | sed 's/{0}{\\small/{0}{/'> $@

table_adjust2.tex: table_adjust.pln
	$(PLAIN2) -tex -fexampl -table=60 table_adjust.pln > $@

table_adjust2.texv: table_adjust.pln
	(echo '% plain2 -table=60 -tex -nopre' table_adjust.pln ;\
	$(PLAIN2) -table=60 -tex -nopre table_adjust.pln ) | ./pln2raw > $@

xx_example_adjust2.tex: example_adjust.pln
	$(PLAIN2) -tex  -fexampl -exam=60 example_adjust.pln > $@

picture_adjust.tex: picture_adjust.pln
	$(PLAIN2) -tex  -nopre  picture_adjust.pln > $@

section_star.texv: section_star.pln
	(echo '% plain2 -fsupsnum -tex -nopre' table_adjust.pln ;\
	$(PLAIN2) -fexampl_star -tex -nopre table_adjust.pln ) | ./pln2raw > $@

section_stars.tex: section_star.pln
	$(PLAIN2) -tex  -fexampl_star section_star.pln > $@

renumber_out.pln: renumber.pln
	$(PLAIN2) -renum  renumber.pln > $@

reference.tex: reference.pln
	$(PLAIN2) -tex -fexampl -ref $< > $@

space_intro.raw: space_intro.pln
	sed 's//^L/' $< | ./pln2raw -n > $@

hint_raw.texv: hint_raw.pln

$(PLAIN2):
	@echo

help_message.raw: help_message
	expand help_message| ./pln2raw  > $@

help_message: $(PLAIN2)
	-$(PLAIN2) -help > $@ 2>&1

.pln.texv:;
	(echo '% plain2 -tex -nopre' $< ;\
	$(PLAIN2) -tex -nopre $< ) | ./pln2raw > $@

.pln.tex: exampl.p2
	$(PLAIN2) -tex -fexampl  $< > $@

.pln.lofi: exampl.p2
	$(PLAIN2) -tex -nolistd -fexampl  $< > $@

.pln.raw:
	./pln2raw -n $< > $@

.obj.eps:
	$(PRTGIF) -eps $<

.SUFFIXES: .pln .tex .raw .lofi .eps .obj .texv

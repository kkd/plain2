#
# pt2htcol.awk: plain2html's parts collection awk-script.
#	by k-chinen@is.aist-nara.ac.jp , 1994
#
# plain2's (with -html) output have "plain2:TABLE" and "plain2:PICTURE"
# sentence. It is mean that output have link to TABLE and PICTURE.
# Therefore, this script collect such sentence and make cutting script.
#
BEGIN{
}
/^<!-- plain2:TABLE/{
	# print "# " $0
	printf "# table %d th, start %d, end %d\n", $3, $4, $5
	printf "head -%d _src| tail -%d > TBL%05d.p\n",$5, $5-$4+1, $3
}
/^<!-- plain2:PICTURE/{
	# print "# " $0
	printf "# picture %d th, start %d, end %d\n", $3, $4, $5
	printf "head -%d _src| tail -%d > PIC%05d.p\n",$5, $5-$4+1, $3
}

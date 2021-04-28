BEGIN {
	# Switch field separator from space to tabulator.
	FS="\t"	
}

{
	print "'''"
	print "[[cmd-" $1 "]]"
	print "****"
	print "*Syntax:* <<cmd-" $1 "," $1 ">>", $2, "+"
	sub("\"\"", "n/a", $3)
	print "*Example:*", $4, "+"
	print "****"
	print "\n" $3, "+"
	print "\n\n"
}

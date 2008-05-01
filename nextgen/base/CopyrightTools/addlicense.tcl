#!/bin/sh
# Start Wish. \
exec tclsh ${0} ${@}

#
#   Add license text 
#
#  addlicense files
#    Prepends the text in License.txt to each of files.
#

#
#   Useful consts.
#
set Licensefile ~fox/license/License.txt

set ticks [clock seconds]
set year  [clock format $ticks -format %Y]
set Copyright \
	"(C) Copyright Michigan State University $year, All rights reserved"

set implementation ".cpp .cc .c .cxx .C" ;# Type of implementation files.
#
#  Read the license text file.
#
set lf [open $Licensefile r]
set license [read $lf]
close $lf

#
#  Process the input files;
#
foreach file $argv {

    puts -nonewline "Working on $file...license  "

    #  Make a temp file for the output:

    set outfile [exec mktemp $file.XXXXXXX]
    set ofd [open $outfile w]
    puts $ofd $license

    # If the file is implementation, put copyright text:

    set type [file extension $file]
    if {[lsearch -exact $implementation $type] >= 0} {
	puts -nonewline " copyright notice"
	puts -nonewline $ofd "static const char* Copyright = "
	puts -nonewline $ofd "\""
	puts -nonewline $ofd $Copyright
	puts  $ofd "\";"
    }
    close $ofd

    # put the original file on the end of outfiule:
    puts -nonewline "Original file "
    exec cat $file  >> $outfile

    # move the new file in place:

    puts -nonewline " install it"
    file rename -force $outfile  $file
    puts done

}

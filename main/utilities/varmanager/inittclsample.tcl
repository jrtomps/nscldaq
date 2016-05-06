#!/bin/bash
# \
    exec $DAQBIN/inittcl "$0" ${1+"$@"}

if {[llength $argv] != 1} {
    puts "Usage: "
    puts "   inittclsample prompt"
    exit -1
}

namespace eval ::prompter:: {
    variable prompt $::argv
}

proc prompt {} {
    puts -nonewline $::prompter::prompt
    flush stdout
}

proc newprompt prompt {
    set ::prompter::prompt $prompt
}

set tcl_prompt1 [list prompt]


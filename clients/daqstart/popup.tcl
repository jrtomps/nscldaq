#!/bin/sh
#
#   pop up a dialog.. this is a shell script rather than a tcl
#   script so we don't have to worry about where wish hides:
#
#
#   start wish \
exec wish ${0} ${@}

proc Usage {} {
    puts "Usage:"
    puts "   popup message ..."
}

wm withdraw .
if {$argc == 0} {
    Usage
    destroy .
    exit 1
} else {
    tk_dialog .noticepopup  "DAQ notice" $argv info 0 "Dismiss"
    destroy .
    exit 0
}
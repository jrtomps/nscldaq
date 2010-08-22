#!/usr/bin/wish
#
#  Threshold manager for the vmusb readout. The idea is that each of the
#  devices that has a threshold will have a threshold file.
#  the file will contain the threshold values one per line.
#  moduleThresholds will be used to edit the thresholds which can then
#  be written back to the original file..where they will be applied on the
#  next start run.
#

package require Tk

set here [file dirname [info script]]
set config [file join ~ config]

source [file join $here moduleThresholds.tcl]



#----------------------------------------------------------------------
#
#  Entry point.  If no file is given on the command line, prompt for
#  one from among *.thresholds in ~/config
#

if {[llength $argv] == 1} {
    set filename [lindex $argv 0]
} else {
    set filename [tk_getOpenFile -defaultextension .thresholds \
	-initialdir $config -filetypes {
	    {{Threshold Files} .thresholds }
	}]
}

# Get the thresholds from the file.  Turn them into a list and allow for the possibility 
# that not all are present or too many are present. Too many are truncated while not enough
# are padded out with zeroes.
#


set f [open $filename r]
set values [read $f]
close $f
set values [split $values "\n"]
set values [lrange $values 0 31]

while {[llength $values] != 32} {
    lappend values 0
}

#
#  Create the main control panel:
#
thresholds .t
.t set $values
pack .t

#
#   Set the save button:
#
button .save -text Save -command save
pack .save

#Save button executes:
#
proc save {} {
    global filename

    set values [.t get]
    set fd [open $filename w]
    foreach value $values {
	puts $fd $value
    }
    close $fd

}
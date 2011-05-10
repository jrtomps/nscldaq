#
#   This is the control panel application for the
#  CAEN V6533 power supply.
# Usage:
#    panel name
# Where
#    name - is the name of the device as known to 
#           Readout.
#

package require Tk

set here [file dirname [info script]]
source [file join $here channel.tcl]

#  Layout the main part of the screen:
#  there are channel widgets for each of the
#  6 channels of the module.
#

for {set i 0} {$i < 6} {incr i} {
    v6533Channel .c$i

}
#  Each row has three widgets:

set row 0
set col 0
for {set i 0} {$i < 6} {incr i} {
    grid .c$i -row $row  -column $col
    incr col
    if {$col > 2} {
	set col 0
	incr row
    }
}
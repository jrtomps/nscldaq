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
#
#  Print the program usage:
#

proc usage {} {
    puts stderr "Usage:"
    puts stderr "   panel name"
    puts stderr "Where:"
    puts stderr "   name - the name of a V6533 device in controlconfig.tcl"

}

# 
#  Produce a row label. This serves to 
#  describe the contents of the channel widgets:
#
# Parameters:
#   wid  - widget name of containing frame.
# Returns:
#   wid
#
proc rowLabel wid {
    frame $wid -relief groove -borderwidth 2
    label $wid.name -text name
    label $wid.set  -text setpoint
    label $wid.v    -text voltage
    label $wid.i    -text current

    grid $wid.name
    grid $wid.set
    grid $wid.v
    grid $wid.i

    return $wid
}

#------------------------------------------------

#  If the parameter count is not right, error exit

if {[llength $argv] != 1} {
    usage
    exit -1
}
set name $argv

label .l -text "V6533 control for $argv"
grid .l -columnspan 3

#  Layout the main part of the screen:
#  there are channel widgets for each of the
#  6 channels of the module.
#

for {set i 0} {$i < 6} {incr i} {
    v6533Channel .c$i -label "Ch $i" -blabel On -bg green \
	-setpoint 0 -actualv 0 -actuali 0

}
#  Each row has three widgets:

set row 1
set col 1
grid [rowLabel .r$row] -row $row -column 0 -sticky n
for {set i 0} {$i < 6} {incr i} {
    grid .c$i -row $row  -column $col
    incr col
    if {$col > 3} {
	set col 1
	incr row
	grid [rowLabel .r$row] -row $row -column 0 -sticky n

    }
}
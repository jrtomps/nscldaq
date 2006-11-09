#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package provide epicsLed 1.0
package require Tk
package require snit
package require led
package require epics

namespace eval controwidget {
    namespace export epicsLed
}

#
#   Epics LED widget.  This is an encapsulation
#   of an led that traces the contents of an epics channel.
#   the LED is on if the channel is nonzero and off otherwise.
#
#  Options:
#      All those for led.
#      -channel   - the channel to use.
#      -showlabel - boolean true to also show the channel name as a label.
#
#

snit::widget controlwidget::epicsLed {
    delegate method * to led
    delegate option * to led except [list -channel -showlabel]
    
    option -channel {}
    option -showlabel 0

    constructor args {
	install led as controlwidget::led $win.led
	$self configurelist $args
	
	set channel $options(-channel)
	if {$channel eq ""} {
	    error "epicsLabel widgets need a -channel on construction"
	}
	epicschannel $channel

	$channel         link ::controlwidget::$channel

	$self configure -variable ::controlwidget::$channel
	grid $win.led -sticky nsew -row 0 -column 0


    }
    #
    #   If the label configuration state changes, then
    #   we need to make/destroy the label widget.
    #

    onconfigure -showlabel enable {
	set old $options(-showlabel) 
	set options(-showlabel) $enable
	
	# Adding the lable when not there.

	if {$enable && !$old} {
	    label $win.label -text $options(-channel)
	    grid $win.label -row 0 -column 1
	}
	# Removing the label when there

	if {!$enable && $old} {
	    destroy $win.label
	}
	# the other two cases are no-ops.
    }
}
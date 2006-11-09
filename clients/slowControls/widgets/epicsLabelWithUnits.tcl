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

#
#  Provide a widget that displays an epics channel along with its
#  .EGU field.  These are really two epics labels that
#  are side by side.
#
package provide epicsLabelWithUnits 1.0


package require Tk
package require snit
package require epics
package require bindDown

namespace eval controlwidget {
    namespace export epicsLabelwithUnits
}

snit::widget ::controlwidget::epicsLabelWithUnits {
    option -channel {}
    delegate option * to hull

    constructor args {
	$self configurelist $args
	set channel $options(-channel)
	if {$channel eq ""} {
	    error "epicsLavelWithUnits must have a -channel on construction."
	}

	epicschannel $channel
	epicschannel ${channel}.EGU
	$channel link       ::controlwidget::$channel
	${channel}.EGU link ::controlwidget::${channel}.EGU

	label $win.value -textvariable ::controlwidget::${channel}
	label $win.units -textvariable ::controlwidget::${channel}.EGU


	grid $win.value -column 0 -row 0 -sticky e
	grid $win.units -column 1 -row 0 -sticky w

	bindDown $win $win
    }
}

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
#  Implements a typeNGo widget that is specialized to epics.
#  In particular, the label part of the typeNGo tracks an epics
#  channel value and the click/<Return> binding will
#  map to setting the value of the channel from the entry widget.
#
# OPTIONS:
#   -channel   - Specifies the epics channel to control/monitor.
#   All other options other than -command are delegated to the
#   typeNGo widget we are based on.
#
# METHODS:
#   All typeNGo methods are supported.
#

package provide epicsTypeNGo 1.0
package require Tk
package require snit
package require typeNGo

package require epics
package require bindDown

namespace eval controlwidget {
    namespace export epicsTypeNGo
}

snit::widget controlwidget::epicsTypeNGo {
    delegate option * to basewidget except -command
    delegate method * to basewidget
    option -channel {}

    variable timerId -1

    constructor args {

	install basewidget as controlwidget::typeNGo $win.tng -command [mymethod onChanged]
	$self configurelist $args

	#  Now deal with getting/binding the channel to the widget.
	#

	set channel $options(-channel)
	if {$channel eq "" } {
	    error "controlwdiget::epicsTypeNGo must have a -channel specification"
	}
	epicschannel $channel
	$channel link ::controlwidget::$channel
	$win.tng configure -textvariable ::controlwidget::$channel

	pack $win.tng -fill both -expand 1

	# try to preload the entry with the setv...
	# Don't try for modicon points.. they don't have a SETV.
	#
	if {[string range $channel 0 1] ne "P#"} {
	    epicschannel $channel.SETV
	    set timerId [after 100 [mymethod loadEntry]]
	}

	bindDown $win $win
    }

    destructor {
	if {$timerId != -1} {
	    after cancel $timerId
	}
    }
    # Called when the entry has been comitted.. we just need to set the epics
    # channel value:
    
    method onChanged {} {
	set newValue [$win.tng Get]
	set channel $options(-channel)
	$channel set $newValue;		# The label will track readback changes.
    }
    #
    #   Called to attempt to load the entry widget with the value of SETV at
    #   start time.  On failure reschedules self.
    #

    method loadEntry {} {
	set channel $options(-channel).SETV
	if {[catch {$channel get} value] || ($value eq "")} {
	    set timerId [after 100 [mymethod loadEntry]]
	    return
	}
	# value is the current setv value:

	$win.tng Set $value
	set timerId -1
    }
}
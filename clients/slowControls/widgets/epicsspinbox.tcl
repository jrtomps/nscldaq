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



#  Provides a spinbox that can be used to control various
#  types of epics devices;  The spinbox can optionally
#  also reflect the reading.  The spinbox can be used
#  as well for enumerated types since all 
#  of the Tk spinbox options are allowed.
#
#  The layout is as follows when monitoring of the set 
#  value is enabled:
#
#  +------------------------------+
#  |      Channel_name            |
#  |    <read value> <units>      |
#  |   [set value]^V              |
#  +------------------------------+
#
# Options:
#    All the options related to a spinbox are accepted.
#    -channel      - Epics channel to control/monitor. Must be 
#                    provided at construction time and cannot be changed.
#
#    -showsetting  - bool, if true, the read value/units are shown.
#                    otherwise, they are not shown.
#
# Methods:
#    Set   - Set the value of the device.
#    Get   - Get the value from the device.
#

package provide epicsSpinbox 1.0
package require Tk     
package require snit
package require epicsLabelWithUnits;		# The label/units.
package require bindDown


namespace eval controlwidget {
    namespace export epicsSpinbox
}

snit::widget ::controlwidget::epicsSpinbox {
    option -channel      {}
    option -showsetting  true
    delegate option * to spinbox

    variable constructing true
    variable read         false

    constructor args {
	install spinbox as spinbox $win.spin -command [mymethod onChange %s]
	$self configurelist $args

	if {$options(-channel) eq ""} {
	    error epicsSpinbox widgets need a -channel on construction
	}

	label $win.label -text $options(-channel)
	::controlwidget::epicsLabelWithUnits $win.feedback -channel $options(-channel)

	$self LayoutWidgets



	set constructing false
	after 100 [mymethod getValue]

    }
    #
    #  Process the -showsetting configuration.
    #
    onconfigure -showsetting value {
	set options(-showsetting) $value
	if {!$constructing} {
	    $self LayoutWidgets;	# Will get done in the constructor initially.
	}
    }
    #  Initially get the value of the channel and stuff it in the
    # spinbox... if we can't get it we'll try again 100ms later.
    #
    method getValue {}  {
	if {!$read} {
	    if {[catch {$options(-channel) get} value]} {
		after 100 [list $self getValue]
	    } else {
		$win.spin set $value
		if {$value ne ""} {
		    set read true
		} else {
		    after 100 [list $self getValue]
		}
	    }
	}
    }

    #  
    #
    #  Called on a <Return> in the spinbox, the user has just typed a new
    # spinbox value...
    #
    method newValue {} {
	set newValue [$win.spin get]
	$self onChange $newValue
    }
    # Called when the value of a spin box has changed.
    # Change the value of the corresponding channel:
    #
    method onChange value {
	$options(-channel) set $value
    }
    #
    #  Layout (optionally unlayout first) the widget.
    #
    method LayoutWidgets {} {
	
	# If it's not the constructor calling us, then un-layout first.
	#
	if {!$constructing} {
	    grid forget $win.label $win.spin
	    if {[$winfo ismapped $win.feedback]} {
		grid forget $win.feedback
	    }
	}
	#  Layout the widgets:

	grid $win.label
	if {$options(-showsetting)} {
	    grid $win.feedback
	}
	grid $win.spin

    }
}

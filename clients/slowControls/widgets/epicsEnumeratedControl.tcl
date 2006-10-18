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



#  This software provides a megawidget for controlling EPICS
#  enumerated devices.  An enumerated devie in EPICS currently
#  consists only of things that have distinct values.
#  Unfortuntately, epics enumerated channels are underpowered with 
#  respect to self description.  What I'd really like to be able to do
#  is ask them questions like:
#  1. How many distinct states do you have.
#  2. What is the list of names that correspond to those states.
#
#  Since I can't do this now, I must get this information at widget
#  construction time from the application.
#  These will be used to configure the underlying radioMatrix.
#
# OPTIONS:
#   -orient   - passed to radioMatrix
#   -columns  - passed to radioMatrix
#   -rows     - passed to radioMatrix.
#   -values   - passed to radioMatrix.
#   -channel  - Epics channel to connect to this control.
#               The label will reflect the channel value and the
#               radio buttons will modify the SETV field for the channel
#               in the database.
# METHODS:
#   Get    - Gets the current requested value.
#   Set    - Sets a new requested value.
#
# NOTES:
#   1. The Gui is implemented in terms of a radioMatrix
#   2. The widget creates variables in the ::controlwidget:: namespace
#      this allows multiple widgets that track the same item to co-exist
#      and do the right thing.
#   3. rows/columns/values constraints hold as for the radioMatrix widget.
#

package provide epicsEnumeratedControl 1.0
package require Tk
package require epics
package require radioMatrix


namespace eval controlwidget {
    namespace export epicsEnumeratedControl
}

snit::widget ::controlwidget::epicsEnumeratedControl {
    option -channel
    option -orient
    option -columns
    option -rows
    option -values

    constructor args {
	$self configurelist $args

	# Set up access to the epics channel.

	set channel $options(-channel)
	if {$channel eq ""  } {
	    error "epicsEnumeratedControl needs a -channel"
	}
	
	epicschannel  $channel
	$channel link ::controlwidget::$channel

	# build up the command to create the radioMatrix:

	set    rmOptions "-variable $channel";    # Track the channel in the label.
	append rmOptions { -command [mymethod onChange]}
	foreach opt [list -orient -columns -rows  -values] {
	    if {$options($opt) ne ""} {
		append rmOptions " $opt [list $options($opt)]"
	    }
	}
	# Create and set the widget>

	eval controlwidget::radioMatrix $win.rm $rmOptions \
	    -variable ::controlwidget::$channel
	pack $win.rm -fill both -expand 1
	after 100 [mymethod updateRadio]

    }
    #  Processes radio button hits.. just turns these around
    #  into settings.
    #
    method onChange  {} {
	set newValue [$win.rm Get]
	if {$newValue ne ""} {
	    set channel $options(-channel)
	    $channel set  $newValue
	}
    }
    # Called, hopefully when the channel is connected to set the
    # initial radio button (if we can).
    #
    method updateRadio {} {
	set channel $options(-channel)

	# If not connected reschedule every 100ms

	if {[catch {$channel get} value] || ($value eq "")} {
	    after 100 [mymethod updateRadio]
	}
	
	$self Set $value
    }

    # Delegate the set method to the radio matrix.

    method Set value {
	$win.rm Set $value

    }
}

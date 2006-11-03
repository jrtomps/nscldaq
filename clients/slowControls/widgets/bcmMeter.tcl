#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#
#  Implements a meter based control for a bcm Epics channel
#  (BCM == Beam Current Monitor).
#
#  A beam current monitor consists of a channel 'channel'
#  with the following interesting fields:
#
#  Name           Purpose
#  channel       The value of the device, a floating point number
#  channel.MRNG  The current range setting of the BCM... this
#                has some roundoff issues and will be mapped to
#                n.0em (for most BCM's n is 1 or 3).
# channel.MSRN   The requested range.
# channel.MRRN   The actual requested range number.
#
#   The widget is laid out as follows:
#
#  +--------------------+
#  | [current value ]   |
#  |  +---------+       |
#  |  | meter   |       |
#  |  |of chan  |       |
#  |  |+/-range |       |
#  |  +---------+       |
#  |  Change Range      |
#  |  [^]    [V]        |
#  +--------------------+
#
#
# Options:
#     -meterheight   - Delegated to meter as -height [static]
#     -meterwidth    - Delegated to meter as -width  [static]
#     -channel       - Epics channel name.           [static]
#     -label         - If supplied overrides the -channel as a label for the
#                      widget.
# Methods
#     get            - Gets the epics channel value.
#     getRange       - Gets the appropriately cooked range.
#     incRange       - increment range value.
#     decRange       - decrement range value.
#
# NOTE:
#   In order to ensure that it is possible to have more
#   than one meter in an application track the same epics
#   channel, we will create  variables (if they don't exist)
#   in the ::controlwidget:: namespace:
#      channel and channel.MRANGE
#   This ensures that one meter does not wipe out the
#   work of another.
#
package provide bcmMeter 1.0

package require meter
package require epics
package require BWidget;      # for ArrowButton.
package require snit
package require bindDown

namespace eval controlwidget {
    namespace export bcmMeter
}

snit::widget  controlwidget::bcmMeter {
    option -meterheight 2i
    option -meterwidth  1.5i
    option -channel {}
    option -label   {}

    constructor args {
        $self configurelist $args


        # Create and bind the epics channels:

        set channel $options(-channel)
        if {$channel eq ""} {
            error "Need an epics channel name to bind to the bcmMeter"
        }
	if {$options(-label) eq ""} {
	    set options(-label) $options(-channel)
	}

        epicschannel $channel
	epicschannel $channel.EGU
        epicschannel $channel.MRNG
        epicschannel $channel.MSRN
        epicschannel $channel.MRRN

        $channel      link ::controlwidget::$channel
	$channel.EGU  link ::controlwidget::$channel.EGU
        $channel.MRNG link ::controlwidget::$channel.MRNG

        #  Construct the widgets:

        label $win.title -text $options(-label)
        label $win.value -textvariable ::controlwidget::$channel
	label $win.units -textvariable ::controlwidget::$channel.EGU
        ::controlwidget::meter $win.meter -height $options(-meterheight) \
                                          -width $options(-meterwidth)   \
                                          -variable ::controlwidget::$channel
        trace add variable ::controlwidget::$channel.MRNG write [mymethod setMeterRange]
        label $win.rangelabel -text {Change Range}
        ArrowButton $win.incrange -command [mymethod incrementRange] -dir top
        ArrowButton $win.decrange -command [mymethod decrementRange] -dir bottom

        # Layout the widgets:

        grid $win.title                 -
        grid $win.value           $win.units
        grid $win.meter                 -
        grid $win.rangelabel            -
        grid $win.incrange        -row 4 -column 0 -sticky w
	grid $win.decrange        -row 4 -column 1 -sticky e

	bindDown $win $win

    }
    #-------------------------- public methods ---------------------------

    # Get the current value of the channel.  This is done by gettint the
    # value of the bound variable
    #
    method get {} {
	set channel $options(-channel)
	return [set ::controlwidget::$channel]
    }
    #  EWxternally bump the ranges up and down:

    method incRange {} {
	$self incrementRange
    }
    method decRange {} {
	$self decrementRange
    }
    #  Retrieve the rang efrom the .mrng part of the channel

    method getRange {} {
	set field $options(-channel).MRNG
	return [set ::controlwidget::$field]
    }
    #--------------------------- Private methods -------------------------
    #
    #  Called when the actual range changes to set the meter range.
    #
    method setMeterRange {name1 name2 op} {
        # If it changed it must have a value..


        set channel $options(-channel)
        set range [set ::controlwidget::$channel.MRNG]

        set goodRange [format %.02e $range]

        # most bcm's have ranges in the form 1.ex 3.ex
        #
        set rangeExponent [expr round(floor(log10($goodRange)))]
        set rangeMantissa [expr round($goodRange/(1e$rangeExponent))]

        if {$rangeMantissa == 3} {
            set majorInterval [format %de%d 1 $rangeExponent]
        } elseif {$rangeMantissa == 1} {
            set majorInterval [format %de%d 5 [expr $rangeExponent -1]]
        } else {
            # Fallback on major tick is 1/6'th the meter range (2*bcm range).

            set majorInterval [expr $goodRange/3]
        }

        $win.meter configure -from [expr -$goodRange] -to $goodRange -majorticks $majorInterval \
                                                                     -minorticks 4

    }
    # Called when the range+ button is clicked.
    # When the actual range changes, the setMeterRange trace
    # will fire to update the graphics.
    #
    #  NOTE: It's possible for the user to click on
    #        the range button prior to the range channel
    #        getting connected... in that case we get an
    #        error from the get of the range and just return.
    #
    method incrementRange {} {
        if {[catch [list $options(-channel).MRRN get] value]} {
            return
        }
        incr value
        catch [list $options(-channel).MSRN set $value];
    }
    # Called when the range- button is clicked.
    # When the actual range changes, the setMeterRange trace
    # will fire to update the graphics.
    #
    method decrementRange {} {
        if {[catch [list $options(-channel).MRRN get] value]} {
            return
        }
        incr value -1
        catch [list $options(-channel).MSRN set $value]
    }

}

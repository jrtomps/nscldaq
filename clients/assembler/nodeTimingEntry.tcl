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



package provide NodeTimingEntry 1.0
package require Tk
package require snit


#  This package provides a megawidget that
#  allows users to enter timing information for a
#  node of the event builder.
#
#  The widget form is as follows:
#
#  +--------------------------------+
#  | [ ] Trigger                    |
#  | Match Window    Match Offset   |
#  | [       ]      [       ]       |
#  +--------------------------------+
# 
#  When Trigger is checked, the two entries are
#  disabled.
#
# OPTIONS
#   -istrigger   (bool)      true if the node is a trigger.
#   -matchwindow (int)       Match window width.
#   -matchoffset (int)       Match window offset.
#
#

snit::widget  nodeTimingEntry {
    option -istrigger    0
    option -matchwindow  1
    option -matchoffset  0

    variable priorWindow 1
    variable priorOffset 0

    constructor args {
	 # Build the widget layout first:

	checkbutton $win.trigger   -text {Trigger}                             \
                                   -variable ${selfns}::options(-istrigger)     \
 	                           -command [mymethod onTriggerStateChange]
	label $win.winlabel  -text {Match Window}
	label $win.offlabel  -text {Match Offset}

	entry $win.winentry  -textvariable ${selfns}::options(-matchwindow)   \
	                     -validate      focusout                         \
                             -validatecommand [mymethod validateWindow %P %s]
	    
	entry $win.offentry  -textvariable ${selfns}::options(-matchoffset)  \
	                     -validate      focusout                         \
	                     -validatecommand [mymethod validateOffset %P %s]



	grid $win.trigger     - -sticky w
	grid $win.winlabel   $win.offlabel
	grid $win.winentry   $win.offentry

	$self configurelist $args
    }
    # Configuration handlers.

    # If the trigger is modified it must not only be saved but
    # onTriggerStateChange called to set the ghosting of the
    # match window/offset appropriately.
    #
    onconfigure -istrigger value {
	if {![string is boolean $value]} {
	    error "-istrigger value must be a boolean and is $value"
	}
	set options(-istrigger) $value
	$self onTriggerStateChange
    }
    #
    #  Validate the matchwindow - must be a positive integer
    #
    onconfigure -matchwindow value {
	if {[$self validateWindow $value]} {
	    set options(-matchwindow) $value
	    set priorWindow $value
	} else {
	    error "-matchwindow value must be a postive integer"
	}
    }
    # The matchoffset must be valid too:
    
    onconfigure -matchoffset value {
	if {[$self validateOffset $value]} {
	    set options(-matchoffset) $value
	    set priorOffset $value
	} else {
	    error "-matchoffset must be an integer"
	}
    }

    #  Local methods:

    # Update the winentry and offentry widget states to reflect
    # the current value of -istrigger.  If that's true,
    # they are disabled otherwise normal:

    method onTriggerStateChange {} {
	if {$options(-istrigger)} {
	    $win.winentry config -state disabled
	    $win.offentry config -state disabled
	} else {
	    $win.winentry config -state normal
	    $win.offentry config -state normal
	}
    }
    
    #  Matching windows must be values that are >= 0.  0 is allowed
    #  but boy I wouldn't do it if I were you.
    #  Returns 1 if value is legal. 0 otherwise.

    method validateWindow {value {prior ""}} {
	if {[string is integer -strict  $value] && ($value > 0) } {
	    set priorWindow $options(-matchwindow)
	    return 1
	} else {
	    $win.winentry delete 0 end
	    $win.winentry insert end $priorWindow
	    return 0
	}
    }
    # Match offsets must be values that are integers.
    # Returns 1 if value is legal, 0 otherwise.
    #
    method validateOffset {value {prior ""}}  {
	if {[string is integer -strict $value]} {
	    set priorOffset $options(-matchoffset)
	    return 1
	} else {
	    $win.offentry delete 0 end
	    $win.offentry insert end $priorOffset
	    return 0
	}
    }
    
}
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

#  This widget provides a Type and Go control.
#  The idea is that there is a label that can be tied
#  to a variable.  Below that is an entry field, 
#  and below that a button.   A click on the
#  button or a key <Enter> in the entry widget
#  Result in the invocation of a command script.
#
#
# Options:
#    All label options are delegated to the label widget.
#    -command is delegated to the button widget
#    -text    is delegated to the button widget.
#
# Methods:
#   Get   - Gets the value of the entry widget.
#   Set   - Sets the value of the entry widget.
#   Invoke- Simulates a push of the buttons.
#
# Bindings:
#  <Enter>  - invokes the pushbutton, when detected in the entry widget.
#
#
package provide typeNGo  1.0
package require Tk
package require snit

namespace eval controlwidget {
}

snit::widget controlwidget::typeNGo {
    delegate option -command to button
    delegate option -text    to button
    delegate option -label   to label as -text
    delegate option  *        to label

    constructor args {
	install label  as label $win.label
	install button as button $win.button
	entry $win.entry

	$self configurelist $args


	grid $win.label  -sticky w
	grid $win.entry  -sticky w
	grid $win.button -sticky w

	# bind <Enter> in the entry so that it calls invoke:

	bind $win.entry <Return> [mymethod Invoke]
    }
    #
    #  Return the contents of the entry widget
    #
    method Get {} {
	return [$win.entry get]
    }
    #
    #  Set the value of the entry widget.
    #
    method Set {value} {
	$win.entry delete 0 end
	$win.entry insert end $value
    }
    #
    # Invokes the button.  This fires any command script
    # That has been bound to the button.
    #
    method Invoke {} {
	$win.button invoke;		# Fire the script.
    }
}
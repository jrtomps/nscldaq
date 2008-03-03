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



package provide EnterReadout 1.0
package require Tk
package require snit


#  The EnterReadout megawidget provides a form that prompts for 
#  the path to the readout program, and the command paramters
#  that will be passed to the program when it is run.
#  
#  The widget consists of an entry field for the path 
#  with  a Browse... button on the right that brings up
#  a tk_openFile dialog, and a second entry field on the
#  bottom that in which the arguments can be entered.
#
#  No shell-like substitutions will be performed on the
#  parameters.
#
# OPTIONS:
#   -path   - Path to the readout program (set or get).
#   -args   - Command arguments to the readout program (set or get).
#
snit::widget EnterReadout {
    option -path
    option -args


    constructor args {
	$self configurelist $args

	label $win.filelabel -text {Filename: }
	entry $win.filename  -textvariable ${selfns}::options(-path)
	button $win.browse   -text {Browse...} -command [mymethod Browse]
	
	label $win.argslabel -text {Arguments: }
	entry $win.args      -textvariable ${selfns}::options(-args)

	# Lay it all out:

	grid $win.filelabel $win.filename $win.browse -sticky w
	grid $win.argslabel $win.args                 -sticky w
	
    }
    # Process the browse button by bringing up a file chooser (modal).

    method Browse {} {
	set newPath [tk_getOpenFile -title {Select Readout program/script} \
			 -filetypes {
			     {{All Files} * }}]

	if {$newPath ne ""} {
	    set options(-path) $newPath
	}
    }


}
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


package require Tk
package require client
package require gdgcontrol
package require gdgwidget
package require configfile
package require BLT

set panelIndex 0
set controllers [list]
set connection ""


proc Reconnect {args} {
    global connection
    global controllers
    while {[catch {controlClient %AUTO%} connection]} {
	tk_messageBox -icon info \
	    -message "The control panel requires readout be running, please start it: $connection"
    }
    foreach object $controllers {
	$object configure -connection $connection
	$object UpdateValues
    }
}

#  addPanel  - creates a new panel for a module.
#
proc addPanel {notebook module connection} {
    global panelIndex
    global controllers

    set name [lindex $module 1]
    puts "Adding $name"
    set w [gdgwidget $notebook.g$panelIndex -title $name]
    set c [gdgcontrol %AUTO% -widget $w -connection $connection \
	                     -name $name -onlost Reconnect]
    lappend controllers $c

    $c UpdateValues

    $notebook insert 0 $name
    $notebook tab configure $name -window $w -fill both 
       
    incr panelIndex
}


#----------------------------------------------------------------
#  Entry point.
#
#  

#  Get connected.
#
Reconnect

blt::tabset .panel

set modules [processConfig ~/config/controlconfig.tcl]
foreach module $modules {
    addPanel .panel $module $connection
}

pack .panel -fill both -expand 1


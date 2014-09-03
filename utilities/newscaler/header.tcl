#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file header.tcl
# @brief Megawidget containing the scaler display header.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide header 1.0
package require Tk
package require snit


##
# header megawidget:
#    +--------------------------------------------------+
#    | Title:   <current title>   Run:  <run num>       |
#    | State:   <current state>                         |
#    +--------------------------------------------------+
#
# OPTIONS:
#  *  -title    - value of the title
#  *  -run      - Run number value.
#  *  -state    - Run state (defaults to -Unknown-)
#
snit::widgetadaptor header {
    option -title
    option -run -default 0
    option -state -default -Unknown-
    
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
        
        ttk::label $win.tlabel -text {Title: }
        ttk::label $win.title -textvariable ${selfns}::options(-title)
        
        ttk::label $win.rlabel -text {Run: }
        ttk::label $win.run    -textvariable ${selfns}::options(-run)
        
        ttk::label $win.slabel -text {State: }
        ttk::label $win.state  -textvariable ${selfns}::options(-state)
        
        grid $win.tlabel $win.title -sticky e
        grid $win.rlabel $win.run   -sticky e
        grid $win.slabel $win.state -sticky e
        
        
    }
}

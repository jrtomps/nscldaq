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
# @file scaleControl.tcl
# @brief Megawidget to control scales of uhm...stuff.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide scaleControl 1.0
package require Tk
package require snit

##
# @class ScaleControl
#    provides controls for axis scales.
#
# APPEARANCE:
# \verbatim
#   +------------------------------------+
#   |  +   -  [V zoom ]  Min:  [     ]   |
#   +------------------------------------+
#
# *  The + - are flush buttons that do a scale up or scale down.
# *  Is a pulldown menu with various zoom options.
# *  Min: - allows you to truncate the axis at the specified mininum value
#
# OPTIONS:
#    - -menulist   - List of items to put on the zoom pulldown menu.
#    - -zoomRange  - See -current as well - this sets the range of menu values
#                    over which the +/- buttons operate.  When the + button is
#                    clicked it will advance the zoom to the next item in the menu
#                    list that is within the range specified by -zoomRange.  Similarly
#                    "-" will  zoom to the prior item.  Suppose for example, the
#                    menulist is [list x1 x2 x4 x8 reset custom].
#                    Suppose that -zoomRange is [list 0 3].  Suppose -current is
#                    1x (item 0).  + will advance the zoom to 2x and - will
#                    do nothing as 0 - 1 is below the -zoomRange low limit of 0.
#   -  -current    - Current menu item text selected.  This can be modified by:
#                    * the configure subcommand.
#                    * the +/- buttons.
#                    * the drop down menu.
#   - -min         -  Value of the minimum.   This can be changed via explicit
#                     configuration or by modifying the Min entry.
#   - -command     -  Script invoked when -current changes.
#   - -mincommand  -  Script invoked when -min is changed.
#
#                    

snit::widgetadaptor ScaleControl {
    component plus
    component minus
    component menu
    component min
    
    option -menulist    [list]
    option -zoomrange   [list]
    option -current     ""
    option -min         0
    option -command     [list]
    option -mincommand  [list]

    typeconstructor  {
        ttk::style configure flat.TButton -relief flat -padx 2
    }
    
    ##
    # construtor
    #    Create/layout the widgets, process our configuration
    #
    constructor args {
        installhull using ttk::frame
        

        
        # Make the widgets.
        
        ttk::button $win.plus  -text + -style flat.TButton -width 1
        ttk::button $win.minus -text - -style flat.TButton -width 1
        
        ttk::menubutton $win.zoombutton -text zoom -menu $win.zoombutton.menu
        menu $win.zoombutton.menu -tearoff 0;    # Filled in on -menulist config.
        
        ttk::label $win.minlbl -text Min:
        ttk::entry $win.min    -width 6
        $win.min insert end 0
        
        # Lay them out:
        
        grid $win.plus $win.minus $win.zoombutton $win.minlbl $win.min
        
        
        $self configurelist $args
    }
}
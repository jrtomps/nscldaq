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
# @file toolbar.tcl
# @brief Provide a toolbar megawidget.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide toolbar 1.0
package require Tk
package require snit

##
# @class toolbar
#
#  A toolbar is a canvas into which tools can be installed.  A toolbar is
#  associated with a canvas into which it's tools get installed when their
#  graphical icons are double-left-clicked.
#
#
snit::widgetadaptor toolbar {
    delegate option * to hull;      # Can be configured like a canvas.
    delegate method * to hull;      # Can even be treated like a canvas.
    
    variable tools;                 # List of tool objects.
    variable nextY 0;               # Offset at which the tool widget is installed.
    
    option -target    "";          # Tools get installed on this canvas.
    
    ##
    # constructor
    #   construct the toolbar  It's going to be up to the caller to decide
    #   where it lives and how it looks (relief e.g.).
    #
    #  -  Install a canvas as a hull.
    #  -  configure.
    #
    constructor args {
        installhull using canvas
        
        $self configurelist $args
    }
    ##
    # _install
    #   Request the tool install itself.
    #
    # @param tool
    #   The tool to install.
    #
    method _install tool {
        if {$options(-target) eq ""} {
            error "No target canvas set"
        }
        
        $tool install $options(-target)
    }
    ##
    # _nextPosition
    #   Figure out where to put a new tool:
    #  @param size - width height of the tool.
    #  @return list - x y coords.
    #
    method _nextPosition size {
        set ht [lindex $size 1]
        set wid [lindex $size 0]
        
        incr nextY $ht
        return [list [expr {$wid/2}] $nextY]
    }
    #--------------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # add
    #   Add a new tool to the toolbar.
    #   This tool will be placed below any previous tool.
    #
    #  @param tool - new tool to add.
    #
    #  TODO: Orient option to allow for horizontal toolbars.
    #
    method add tool {
        set coords [$self _nextPosition [$tool size]]
        
        $tool configure -canvas $win
        $tool drawat {*}$coords
        
        #  Add a double click event to install the tool:
        
        set id [$tool getId]
        
        $win bind $id <Double-Button-1> [mymethod _install $tool]

        lappend tools $tool        
    }
    
    
}
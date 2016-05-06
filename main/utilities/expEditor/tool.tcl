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
# @file tool.tcl
# @brief Define interface for a tool.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide tool 1.0
package require snit

##
# @class Tool
#
#  A tool is an entity that can live in a tool bar.  It consists of a DaqObject
#  and an installer.  The DaqObject represents the object in the toolbar while
#  the installer is capable of putting that tool on a canvas when requested by
#  events in the toolbar and hooking in canvas behavior.
#
#  The installer is an object that must implement an install subcommand.
#  The install command has the form:
#
#   $installer install object from to
#
#  Where:
#    - object is the DaqObject to install (in general this gets cloned).
#    - from is a dict that contains the following key/values:
#      *  canvas - toolbar canvas.
#      *  x      - x Coordinate of the DaqObject on the canvas.
#      *  y      - y Coordinate of the DaqObject on the canvas.
#    - to is the canvas in which to install the cloned DaqObject.
#
# @note - it's not unusual for the DaqObject itself to actually be a compound
#         object containing the data of the object as well as the object's icon.
#
snit::type tool {
    component object;                  # DaqObject displayed on toolbar.
    component installer;               # Knows how to install the object.

    delegate option -canvas       to object
    
    delegate method drawat        to object
    delegate method moveto        to object
    delegate method moveby        to object
    delegate method addtag        to object
    delegate method rmtag         to object
    delegate method tags          to object
    delegate method getPosition   to object
    delegate method getId         to object
    delegate method size          to object
    
    ##
    # constructor
    #   Construct the object
    # @param args - [lindex $args 0] is object [lindex $args 1] is installer.
    #
    constructor args {
        if {[llength $args] != 2} {
            error "Constructing tool - must have only object and installer"
        }
        install object  using lindex $args 0
        install installer using lindex $args 1
    }
    
    #--------------------------------------------------------------------------
    # Public methods
    
    ##
    # install
    #    Toolbar calls this install method We just marshall up the
    #    parameters. 
    #
    # @param c - Target canvas.
    #
    method install {c} {
        set sc [$object cget -canvas]
        set xy [$object getPosition]
        $installer install \
            $object \
            [dict create canvas $sc x [lindex $xy 0] y [lindex $xy 1] ] $c
         
    }

}
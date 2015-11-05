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
# @file ObjectInstaller.tcl
# @brief Installer for object like DAQ objects.
#
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide objectInstaller 1.0
package require snit
package require properties
package require propertyEditor

##
# @class  ObjectInstaller
#
#   Provides installation from a toolbar to a canvas for DaqObject
#   related things:
#   - Puts the object on the destination canvas next to the tool (to its right).
#   - Enables dragging the object around.
#   - Enables double-click to bring up a property list for the object.
#
snit::type ObjectInstaller {
    
    #---------------------------------------------------------------------------
    # Public methods
    #
    
    ##
    # _drag
    #    Respond to an object drag by asking the object to draw itself elsewhere:
    #
    # @param object Object being dragged.
    # @param x      New x coordinate of object.
    # @param y      New y coordinate of object.
    #
    method _drag {object x y} {
        $object moveto $x $y
    }
    ##
    # _editProperties
    #
    #   Brings up the property editor for the object.
    #
    # @param object Object whose properties are being edited.
    #
    method _editProperties object {
        set props [$object getProperties]
        propertyDialog .p \
            -wintitle {Edit Properties} -title Properties: -proplist $props
        
        update idletasks
        
        .p modal
        destroy .p
    }
    
    ##
    # install
    #  Install an object to a destination canvas.
    #  - Clone the object.
    #  - Draw it on the destination canvas
    #  - set up the event handling.
    #
    # @param object - object to clone/install
    # @param from   - dict containing canvas, x, y of tool.
    # @param to     - canvas to install the clone on.
    #
    # @return the newly created object.
    #
    method install {object from to} {
        set newObject [$object clone]
        
        
        #  put the object its width from the left edge at the height of the tool:
        
        set y [dict get $from y]
        set x [lindex [$object size] 0]

        $newObject configure -canvas $to
        $newObject drawat $x $y
        
        #  Now set up the object's behavior in the GUI.
        
        set id [$newObject getId]
        
        $to bind $id <B1-Motion> [mymethod _drag $newObject %x %y ]; # Drag.
        $to bind $id <Double-Button-1> [mymethod _editProperties $newObject]
        
        return $newObject
    }
}
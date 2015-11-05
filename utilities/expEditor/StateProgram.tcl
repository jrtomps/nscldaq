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
# @file StateProgram.tcl
# @brief Encapsulate state program data and a GUI element.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stateProgram 1.0
package require snit
package require Tk
package require stateProgramData
package require daqObject
package require img::png

##
# @class StateProgram
#   Wraps a DaqObject that has the icon for a state program with a
#   StateProgramdata object that has the data.   Appropriate command
#   and option forwarding are done to make this all hang together.
#

snit::type StateProgram {
    component data
    component gui
 
    delegate option -provider to data
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
    delegate method getProperties to data
    delegate method addSink       to data
    delegate method clearSinks    to data
    delegate method rmSink        to data
    delegate method getSinks      to data
    
    delegate method drawat        to gui
    delegate method moveto        to gui
    delegate method moveby        to gui
    delegate method addtag        to gui
    delegate method rmtag         to gui
    delegate method tags          to gui
    delegate method getPosition   to gui
    delegate method getId         to gui
    delegate method size          to gui

    
    ##
    # typeconstructor
    #   Create the image that will be used as the object's icon:
    
    typeconstructor {
        image create photo StateProgramIcon -format png \
            -file [file join [file dirname [info script]] program.png] 
    }
    
    ##
    # constructor
    #   Construct the object. This just means constructing the components.
    #
    constructor args {
        install data using StateProgramData %AUTO%
        install gui  using DaqObject %AUTO% -image StateProgramIcon
        
        $self configurelist $args
    }
    
    ##
    # destructor
    #
    destructor {
        $data destroy
        $gui  destroy
        
    }
    ##
    # _replaceData
    #   Used in a clone operation to replace the data with the data to be copied.
    #
    # @param new data object.
    #
    method _replaceData newData {
        $data destroy
        set data $newData
    }
    
    ##
    # clone
    #   Create/return a copy of self
    #
    method clone {} {
        set newObject [StateProgram %AUTO%]
        $newObject _replaceData [$data clone]
        
        return $newObject
    }
}

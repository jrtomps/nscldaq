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
    delegate method bind          to gui

    variable inRingObj ""
    variable outRingObj ""
    
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
    #---------------------------------------------------------------------------
    # Private methods
    
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
    # _getProperty
    #   Return the value of a named property from an object that supports
    #   the property list API.
    #
    # @param propName  - Name of the properties.
    #
    method _getProperty {object propName} {
        set props [$object getProperties]
        set prop  [$props find $propName]
        return [$prop cget -value]
        
    }
 
    ##
    # _setProperty
    #   sets a named propert on an object.
    #
    # @param object   - object to modify.
    # @param propName - Name of the property.
    # @param value    - new property vale.
    #
    method _setProperty {object propName value} {
        set props [$object getProperties]
        set prop  [$props find $propName]
        $prop configure -value $value
    }
    ##
    # _getUri
    #    Return the URI associated with a ring buffer object.
    #
    # @param ring - the ring buffer object.
    #
    method _getUri ring {
        set host     [$self _getProperty $ring host]
        set ringName [$self _getProperty $ring name]
        
        return tcp://$host/$ringName
    }
    ##
    # _getName
    #   Return the name of a ringbuffer.
    #
    # @param ring - ring buffer object.
    #
    method _getName ring {
        return [$self _getProperty $ring name]
        
    }
    #---------------------------------------------------------------------------
    # Public methods
    
    ##
    # clone
    #   Create/return a copy of self
    #
    method clone {} {
        set newObject [StateProgram %AUTO%]
        $newObject _replaceData [$data clone]
        
        return $newObject
    }
    ##
    # type
    #   Return the object type.
    #
    method type {} {
        return state-program
    }
    ##
    # connect
    #   Called when the program is connected to something.
    #
    #  @param direction - from if we are the source of the connection and to if
    #                     we are the sink end of the connection.
    #  @param object    - Who we are being connected to.
    #
    method connect {direction object} {
        if {[$object type] ne "ring"} {
            error "State programs can only be connected to rings."
        }
        if {$direction eq "from"} {
            set property {Output Ring}
            set value [$self _getUri $object]
            set outRingObj $object
        } elseif {$direction eq "to" } {
            set property {Input Ring}
            set value [$self _getName $object]
            set inRingObj $object
        } else {
            error "Invalid direction on connect"
        }
        $self _setProperty $self $property $value
    }
    ##
    # disconnect
    #   Called when the program is disconnected from an object
    #
    #  @param object
    #
    method disconnect object {
        
        set objUri  [$self _getUri $object]
        set objName [$self _getName $object]
        
        set sink [$self _getProperty $self {Output Ring}]
        set source [$self _getProperty $self {Input Ring}]
        
        # Take appropriate action depending on whether this is the from or to obj.
        
        if {$object eq $outRingObj} {
            $self _setProperty $self {Output Ring} {}
            set outRingObj ""
        } elseif {$object eq $inRingObj} {
            $self _setProperty $self {Input Ring} {}
            set inRingObj ""
        } else {
            error "$objName is not connected to [$self _getProperty $self name]"
        }
    }
    ##
    # isConnectable
    #   Determine if the object can accept the specified connection type:
    #
    # @param direction - from or to indicating whether we are a source or sink.
    # @return boolean true if the requested connection direction is acceptable.
    #
    method isConnectable direction {
        if {$direction eq "from" } {
            set obj $outRingObj
        } elseif {$direction eq "to"} {
            set obj $inRingObj
        } else {
            error "Ivalid connection direction: $direction"
        }
        
        # Connections are allowed if there is no current connection in that
        # direction:
        
        return [expr {$obj eq ""}]
    }
}

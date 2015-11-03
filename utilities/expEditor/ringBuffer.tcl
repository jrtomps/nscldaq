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

package provide ringBuffer 1.0
package require snit
package require properties



##
# @file ringBuffer.tcl
# @brief Describes a ring buffer object.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# @class ringBuffer.tcl
#
#   Provides the information associated with a ring buffer.  The ring buffer
#   information comes in two bits
#   * A property list contains the name of the ring buffer and the host on which
#     it is located.
#   * Internal data such as options and getter/setter methods have the
#     provider and client data (these are typically object names for the
#     programs that fill these roles).
#
#
snit::type RingBuffer {
    option -provider
    

    variable properties ""
    variable sinks      [list]

    
    ##
    # constructor
    #   Build the property list.  Initially all entries are empty.
    #   no validation is required.
    #
    constructor args {
        set host [property %AUTO% -name host]
        set name [property %AUTO% -name name]
        
        set properties [propertylist %AUTO%]
        $properties add $host
        $properties add $name
        
        $self configurelist $args
    }
    ##
    #  destructor
    #    destroy the property and its properties.
    #
    destructor {
        set props [$properties get]
        foreach property $props {
            $property destroy
        }
        $properties destroy
    }
    #-------------------------------------------------------------------------
    # Methods not intended for general use.
    #
    

    #-------------------------------------------------------------------------
    #  Public methods
    
    ##
    # getProperties
    #   Get the property list.
    #
    method getProperties {} {
        return $properties
    }
    ##
    # addSink
    #   Add a new sink object.
    #
    # @param sink  - object to add.
    #
    method addSink sink {
        lappend sinks $sink
    }
    ##
    # clearSinks
    #   Remove all sinks
    #
    method clearSinks {} {
        set sinks [list]
    }
    ##
    # rmSink
    #   Remove a single sink
    #
    # @param sink - sink to remove.
    #
    method rmSink sink {
        set sinkIdx [lsearch -exact $sinks $sink]
        if {$sinkIdx != -1} {
    
            set sinks [lreplace $sinks  $sinkIdx $sinkIdx]
        }
    }
    ##
    # getSinks
    #
    # @return list of sink program.s
    #
    method getSinks {} {
        return $sinks
    }
    ##
    # clone
    #   Method to clone this object.
    #
    # @return new object name.
    #
    method clone {} {
        set newObj [RingBuffer %AUTO% -provider $options(-provider)]
        
        # Copy sinks.
        
        foreach sink $sinks {
            $newObj addSink $sink
        }
        # Copy properties:
        
        set newProps [$newObj getProperties]
        
        $properties foreach property {
            set name [$property cget -name]
            set value [$property cget -value]
            
            set newProp [$newProps find $name]
            $newProp configure -value $value
        }
        return $newObj
    }
}
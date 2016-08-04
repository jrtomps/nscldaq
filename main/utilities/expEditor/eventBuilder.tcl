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
# @file eventBulder.tcl
# @brief Encapsulate the data associated with an event builder.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide eventBuilder 1.0
package require snit
package require properties

##
# @class TsPolicy
#    Snit validator for timestamp policy values.
#
snit::type TsPolicy {
    typemethod validate value {
        if {$value ni [list earliest latest average]} {
            errir "Invalid timestamp property $value"
        }
        return $value
    }
}

##
# @class EventBuilder
#
#   A class that encapsultes the properties associated with an event builder.
#   See the constructor's stocking of the properties component for info
#   about which properties those are See also
#   https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Event_builder_schema#Event-builder-directory-variables
#   for a full description of properties and their meaning.  If you don't have
#   access to that URL, enable documentation building on NSCLDAQ and look at
#   the vardbevb(5daq) man page it builds (also findable via the HTML
#   docs at DAQROOT/share/html/index.html).

snit::type EventBuilder {
    component properties

    delegate option -changecmd to properties
    
    ##
    # constructor
    #    install the properties component and stock it.
    #
    constructor args {
        install properties using propertylist %AUTO%
        
        # Create/install properties:
        
        $properties add [property %AUTO% -name name]
        $properties add [property %AUTO% -name host]
        $properties add [property %AUTO% -name servicePrefix -value ORDERER]
        $properties add [property %AUTO% -name serviceSuffix]
        $properties add [property %AUTO% \
            -name coincidenceInterval -validate [snit::integer %AUTO% -min 1] \
            -value 1]
        $properties add [property %AUTO% \
            -name build -validate snit::boolean -value true]
        $properties add [property %AUTO% \
            -name timestampPolicy -value earliest -validate TsPolicy]
        $properties add [property %AUTO% \
            -name sourceId -value 0 -validate [snit::integer %AUTO% -min 0]]
        $properties add [property %AUTO% -name ring -editable 0]
        
        $self configurelist $args
        
    }
    ##
    # destructor - get rid of the properties in the properrty list and destroy
    #              the list itself:
    
    destructor {
        set props [$properties get]
        foreach p $props {
            $p destroy
        }
        $properties destroy
    }
    ##
    # getProperties
    #   Return the properties for examination/modification:
    #
    # @return property list
    method getProperties {} {
        return $properties
    }
    ##
    # clone
    #   Clone this object producing an exact functional
    #   duplicate.
    #
    # @return copy self
    #
    method clone {} {
        set newObj [EventBuilder %AUTO%]
        
        # Copy property values:
        
        set newProps [$newObj getProperties]
        $properties foreach property {
            set name  [$property cget -name]
            set value [$property cget -value]
            
            set target [$newProps find $name]
            $target configure -value $value
        }
        
        return $newObj
    }
    
}
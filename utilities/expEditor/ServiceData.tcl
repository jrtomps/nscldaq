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
# @file ServiceData.tcl
# @brief <brief purpose>
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide serviceData 1.0
package require snit
package require properties

##
# @class ServiceData
#
#  Contains the data that is associated with a service program.
#  Service programs cannot be connected to data flows or the state machine.
#  Service programs only have a name, a host and a command string.
#
snit::type ServiceData {
    component propertylist;              # The properties in the data.
    delegate option -changecmd to propertylist
    
    ##
    # constructor
    #   Create a property list and stock it with the standard properties:
    #
    constructor args {
        install propertylist using propertylist %AUTO%
        
        $propertylist add [property %AUTO% -name name]
        $propertylist add [property %AUTO% -name host]
        $propertylist add [property %AUTO% -name path]
        
        $self configurelist $args
        
    }
    ##
    # destructor
    #   Destroy properties and the list:
    #
    destructor {
        $propertylist foreach property {
            $property destroy
        }
        $propertylist destroy
    }
    
    ##
    # getProperties
    #    Return the current property set:
    #
    # @return propertylist
    #
    method getProperties {} {
        return $propertylist
    }
    ##
    # clone
    #   Produce a duplicate of this object.  Properties are also cloned.
    #
    # @return cloned copy of self.
    #
    method clone {} {
        set newObj [ServiceData %AUTO%]
        set newProps [$newObj getProperties]
        
        $propertylist foreach prop {
            set name [$prop cget -name]
            set value [$prop cget -value]
            
            set np [$newProps find $name]
            $np configure -value $value
        }
        
        return $newObj
    }
}

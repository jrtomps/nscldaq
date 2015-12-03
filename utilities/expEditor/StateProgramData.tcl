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
# @file StateProgramData.tcl
# @brief Contains the data for a state program.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stateProgramData 1.0
package require snit
package require properties

##
# @class
#   StateProgramData
#
#   Encapsulates the property list for a state aware program.
#
snit::type StateProgramData {
    component properties
    
    ##
    # constructor
    #   Create the propertylist.
    #
    constructor args {
        install properties using propertylist %AUTO%
        
        $properties add [property %AUTO% \
            -name name]
        $properties add [property %AUTO% \
            -name enable -value true -validate snit::boolean]
        $properties add [property %AUTO% \
            -name standalone -value false -validate snit::boolean]
        $properties add [property %AUTO% -name path ]
        $properties add [property %AUTO% -name host ]
        $properties add [property %AUTO% -name {Input Ring} -editable 0]
        $properties add [property %AUTO% -name {Output Ring} -editable 0]  
    }
    ##
    # destructor
    #   Clean up properties an list:
    #
    destructor {
        $properties foreach property {
            $property destroy
        }
        $properties destroy
    }
    #------------------------------------------------------------------------
    #  Public methods
    #
    
    ##
    # getProperties
    #   Return the property list:
    
    method getProperties {} {
        return $properties
    }
    ##
    #   clone
    #
    #  produce a duplicate of this object
    #
    
    method clone {} {
        set newObj [StateProgramData %AUTO%]
        
        ## Propagate current property values:
        
        set newprops [$newObj getProperties]
        $properties foreach prop {
            set newprop [$newprops find [$prop cget -name]]
            $newprop configure -value [$prop cget -value]
        }
        return $newObj
    }
    
    
}

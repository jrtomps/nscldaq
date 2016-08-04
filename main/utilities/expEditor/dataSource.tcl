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
# @file dataSource.tcl
# @brief Encapsulates the properties of a data source.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide dataSource 1.0
package require snit
package require properties
package require daqObject

##
# @class IntegerList
#   Validator  for lists of integers.
#
snit::type IntegerList {
    typemethod validate value {
        if {![string is list $value]} {
            error "Validation failed '$value' must be a valid Tcl list of integers"       
        }
        foreach v $value {
            if {![string is integer -strict $v]} {
                error "Validation failed: '$v' from '$value' is not a valid integer"
            }
        }
        return $value
    }
}

##
# @class DataSource
#    Encapsulates the properties of a data source.  These properties are the name
#    of the data source (presumed qualified by the event buider it's connected
#    to), and the variables that describe the data source.  Those variables
#    are documented at:
#    https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Event_builder_schema#Event-builder-data-source-variables
#
#    This link is an internal NSCL Link.  If you do not have access to that link,
#    these variables are also documented in the online documentation produced by
#    an NSCLDAQ build with documentation enabled.  In that case look at the
#    vardbevb(5daq) manpage (also findable via the HTML
#   docs at DAQROOT/share/html/index.html).
#
#
snit::type DataSource {
    component properties
    
    delegate option -changecmd to properties
    
    ##
    # constructor
    #   Install the property list and create/add the properties to it.
    #
    constructor args {
        install properties using propertylist %AUTO%
        
        # Figure out the ringFragmentSource path:
        
        set here [file dirname [info script]]
        if {[array names ::env DAQBIN] eq "DAQBIN"} {
            set bindir $::env(DAQBIN)
        } else {
            set bindir [file normalize [file join $here ../bin]]
        }
        set fragsrc [file join $bindir ringFragmentSource]
        
        # Add properties to the list:
        
        $properties add [property %AUTO% -name name]
        $properties add [property %AUTO% -name host]
        $properties add [property %AUTO% -name path -value $fragsrc] 
        $properties add [property %AUTO% -name info]
        $properties add [property %AUTO% -name ids -validate IntegerList]
        $properties add [property %AUTO% -name ring -editable 0]
        $properties add [property %AUTO% \
            -name defaultId -validate [snit::integer %AUTO% -min 0] -value 0]
        $properties add [property %AUTO% -name timestampExtractor]
        $properties add [property %AUTO% \
            -name expectBodyHeaders -validate snit::boolean -value true]
        
        $self configurelist $args
    }
    
    destructor {
        $properties foreach p {
            $p destroy
        }
        $properties destroy
    }
    
    ##
    # getProperties
    #   Expose property list to clients.
    #
    # @return property list.
    #
    method getProperties {} {
        return $properties
    }
    ##
    # clone
    #   Return a duplicate of self.
    #
    # @return DataSource - that is a copy of $self.
    #
    method clone {} {
        set newObj [DataSource %AUTO%]
        
        # Copy properties:
        
        set newProps [$newObj getProperties]
        $properties foreach prop {
            set name [$prop cget -name]
            set v    [$prop cget -value]
            
            [$newProps find $name] configure -value $v
        }
        
        return $newObj
    }
}

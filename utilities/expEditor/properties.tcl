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
# @file properties.tcl
# @brief Tcl code to implement properties and property lists.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# A property is a triple that contains:
#
#   *  A name (property name)
#   *  A value (property value)
#   *  A validator that can provide a data type to the property.
#

package provide properties 1.0
package require snit

##
# @class property
#    Defines a single property:
#
# OPTIONS
#    -name     - Name of the property.
#    -value    - Property value.
#    -validate - validation object for value (snit::validation object).

snit::type property {
    option -name -readonly 1
    option -value -default "" -configuremethod _validate
    option -validate -default ""
    
    constructor args {
        $self configurelist $args
        
        if {$options(-name) eq ""} {
            error "Property constructor -- properties must have a -name"
        }
    }
    ##
    # _validate
    #   If there's a validation use it to check the validity of the
    #   a value.
    #
    # @param optname - name of option to be validated (-value).
    # @param value   - proposed value.
    #
    #   If value is acceptable the option is updated otherwise an error is raised.
    method _validate {optname value} {
        if {$options(-validate) eq ""} {
            set options($optname) $value
        } else {
            set validator $options(-validate)
            set options($optname) [$validator validate $value]
        }
    }
}
##
# @class propertylist
#    List of properties.
#
# METHODS:
#    get   - Returns he current property list.
#    add   - Adds a new property to the list.
#    clear - Clear the property list.
#    foreach - Execute a script for each property in the list.
#
snit::type propertylist {
    variable props [list]
    
    ##
    # get
    #   Return the list of properties:
    #
    method get {} {
        return $props
    }
    ##
    #  add
    #    Append a property to the list:
    #
    # @param property  - property object to append.
    method add {property} {
        lappend props $property
    
    }
    ##
    #  clear
    #
    # Clear the property list.  Note that the properties themselves are not
    # destroyed.
    #
    method clear {} {
        set props [list]
    }
    ##
    #  foreach
    #    Iterate over all properties in the list applying a script to each.
    # @param var - variable that will receive the property object in script's
    #              stack level.
    # @param script - script to execute
    #
    method foreach {var script} {
        upvar 1 $var property
        foreach property $props {
            uplevel 1 $script
        }
    }
}


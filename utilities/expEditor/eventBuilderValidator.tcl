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
# @file eventBuilderValidator.tcl
# @brief Validate all state program configurations
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide eventBuilderValidator 1.0

namespace eval ::Validation {}

##
# ::Validation::validateEventBuilders
#
#   Validates a list of event builders:
#   *  Event builders must have names.
#   *  Event builders must be assigned to a host.
#   *  Event builders must have an output ring.
#
# TODO: - event builders must have at least one data source.
#
# @param builders - list of event builder objects.
# @return list - to do list to fix any found problems.
#
proc ::Validation::validateEventBuilders builders {
    set result [list]
    set nonames 0

    foreach evb $builders {
	set p [$evb getProperties]
	set name [[$p find name] cget -value]
	if {$name eq ""} {
	    incr nonames
	    set name -no-name-
	}
	if {[[$p find host] cget -value] eq ""} {
	    lappend result "Event builder $name has not been allocated to a host"
	}
	if {[[$p find ring] cget -value] eq ""} {
	    lappend result "Event builder $name has not been given an output ring"
	}
    }

    if {$nonames} {
	lappend result "There are event builders that have not been given a name"
    }
    return $result
}
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
# @file stateProgramValidator.tcl
# @brief Validate all state program configurations
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stateProgramValidator 1.0
namespace eval ::Validation {}

##
# ::Validation::validateStatePrograms
#
#    *  State programs must have names.
#    *  State Programs must be allocated to a host.
#    *  State Programs must have a program path.
#    *  Warn if state programs don't have either an input or output ring.
#
# @param programs - list of state program objects to check.
# @return list    - To do list to fix any problems found.
#
proc ::Validation::validateStatePrograms programs {
    set nameless 0
    set result [list]

    foreach sp $programs {
	set p [$sp getProperties]
	set name [[$p find name] cget -value]

	if {$name eq ""} {
	    incr nameless
	    set name -no-name-
	}
	if {[[$p find host] cget -value] eq ""} {
	    lappend result "The state program $name has not been allocated to a host"
	}
	if {[[$p find path] cget -value] eq ""} {
	    lappend result "The state program $name has not been given a program path"
	}
	set inring [[$p find {Input Ring}] cget -value]
	set outring [[$p find {Output Ring}] cget -value]
	if {($inring eq "") && ($outring eq "")} {
	    lappend result "Warning - the state program $name has neither an input nor an output ring"
	}
		     
	
    }

    if {$nameless} {
	lappend result "There are state programs that have not been named"
    }
    return $result
}
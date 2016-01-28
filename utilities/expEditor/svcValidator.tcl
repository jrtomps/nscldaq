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
# @file svcValidator.tcl
# @brief Validate all services.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide serviceValidator 1.0

namespace eval ::Validation {}
##
# ::Validation::validateServices
#   For each service provided, checks that:
#   - The service has a name.
#   - The service has a path.
#   - The service has been allocated to a host.
#
#  With the exception of nameless services each problem results in an error
#  added to the result list. If there are several services without a name, however,
#  these messages get collapsed to a single error.
#
# @param svcs  - list of service objects.
# @return list - possibily empty list of validation failure messages.
#
proc ::Validation::validateServices svcs {
    set namelessCount 0
    set result [list]

    foreach svc $svcs {
	set p [$svc getProperties]
	set name [[$p find name] cget -value]
	if {$name eq ""} {
	    incr namelessCount
	    set name -no-name-
	}
	if {[[$p find host] cget -value] eq ""} {
	    lappend result "Service $name has not been assigned to a host"
	}
	if {[[$p find path] cget -value] eq ""} {
	    lappend result "Service $name has not been given a path."
	}
    }

    if {$namelessCount } {
	lappend result "There are services that have not been named"
    }
    return $result
}
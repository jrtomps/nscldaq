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
# @file ringValidator.tcl
# @brief Validate all ringBuffers
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ringValidator 1.0

namespace eval ::Validation {}

##
# ::Validation::validateRings
#
#    Validates ring buffers
#    *   Ringbuffers must have a name
#    *   Ringbuffers must be assigned to a host.
#
# @param rings - ring buffers to validate.
# @return list - (possibly empty) of to dos to fix the problems found
#
proc ::Validation::validateRings rings {
    set namelessCount 0
    set result        [list]
    array set names [list]
    
    foreach ring $rings {
	set p [$ring getProperties]
	set name [[$p find name] cget -value]
	if {$name eq ""} {
	    incr namelessCount
	    set name -no-name-
	}
	set host [[$p find host] cget -value]
	if {$host eq ""} {
	    lappend result "Ring $name has not been assigned to a host"
	}
	if {($name ne "") && ($host ne "") && ([array names names $name@$host] ne "")} {
	    lappend result "There is more than one ring $name@$host"
	} else {
	    set names($name@$host) $name@$host
	}
      }

    if {$namelessCount} {
	lappend result "There are ring buffers that have not been named"
    }

    return $result
}
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
# @file dataSourceValidator.tcl
# @brief Validate all Data source configuration
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide dataSourceValidator 1.0

namespace eval ::Validation {}


##
# Validation::validateDataSources
#
#   Each data source:
#   *  Must have a name.
#   *  Must be connected to an eventbuilder.
#   *  Must have been allocated to a host.
#   *  Must have a source ring.
#   *  Warns if therre is no timestamp extractor lib.
#
# @param dataSources - list of data source objects.
# @return list - of things to do to fix detected problems.
#
proc ::Validation::validateDataSources dataSources {
    set unamed 0
    set result [list]

    foreach src $dataSources {
	set p [$src getProperties]
	set name [[$p find name] cget -value]
	if {$name eq ""} {
	    incr unamed
	    set name -no-name-
	}

	if {[$src getEventBuilder] eq ""} {
	    lappend result "Data source $name is not connected to an event builder"
	}
	if {[[$p find host] cget -value] eq ""} {
	    lappend result "Data source $name has not been allocated to a host"
	}
	if {[[$p find ring] cget -value] eq ""} {
	    lappend result "Data source $name has no input ringbuffer"
	}
	if {[[$p find timestampExtractor] cget -value] eq ""} {
	    lappend result "Warning data source $name has no timestamp extractor"
	}
    }

    if {$unamed} {
	lappend result "There are data sources that have not been given a name"
    }
    return $result
}
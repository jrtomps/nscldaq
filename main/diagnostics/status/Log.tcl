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
# @file Log.tcl
# @brief Emit a log message
# @author Ron Fox <fox@nscl.msu.edu>
#

#
#  Assume this program is installed in e.g. DAQBIN. Don't assume the
#  DAQ env variables are setup however.

set here [file dirname [info script]]
set libdirs [file normalize [file join $here .. TclLibs]]


lappend auto_path $libdirs

package require statusMessage
package require portAllocator

#  Constants:

set legalSeverities [list DEBUG INFO WARNING SEVERE DEFECT]
set service  StatusAggregator

##
#  Output usage string to stderr:
#
proc usage {} {
    puts stderr Usage:
    puts stderr "   Log  Application Severity message"
    
}

#------------------------------------------------------------------------------
#  Entry point

# Check usage:

if {[llength $argv] != 3} {
    usage
    exit -1
}

# Extract the parameters.  Check the validity of the severity:

set app [lindex $argv 0]
set sev [lindex $argv 1]
set msg [lindex $argv 2]

#  Nasty issue that this must be kept up to date!!!


if {$sev ni $legalSeverities} {
    puts stderr "Severity of message was $sev and must be one of {[join $legalSeverities ", "]}"
}

#  Figure out the URI of the local status aggregator.  We're going to look for
#  first as root (production) and then as the user we're running under
#  (test and CI).

portAllocator create pm
set port [pm findServer StatusAggregator root]
if {$port eq ""} {
    set port [pm findServer StatusAggregator $::tcl_platform(user)]
}

if {$port eq ""} {
    puts stderr "Unable to locate the local status aggregator service"
    exit -1
}

set uri tcp://localhost:$port

#  Make the logger and send the message:

set logger [LogMessage create $uri $app]
$logger Log $sev $msg

LogMessage destroy $logger
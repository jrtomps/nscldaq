#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  tclTests.tcl 
# @author Jeromy Tompkins 
#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
::tcltest::configure -testdir [file dirname [file normalize [info script]]]
tcltest::testConstraint false 0
eval ::tcltest::configure $argv

# add hook to access whether tests failed for generating a meaningful return code
set ::failures [list]
proc ::tcltest::cleanupTestsHook {} {
  variable numTests
  lappend ::failures [expr {$numTests(Failed) >0}]
}

::tcltest::runAllTests

puts $::failures
foreach job $::failures {
  if {$job > 0} {
    exit 1
  }
}

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

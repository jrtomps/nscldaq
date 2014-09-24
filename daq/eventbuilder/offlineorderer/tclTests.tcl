#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
::tcltest::configure -testdir [file dirname [file normalize [info script]]]
tcltest::testConstraint false 0
eval ::tcltest::configure $argv

# add hook to access whether tests failed for generating a meaningful return code
proc ::tcltest::cleanupTestsHook {} {
  variable numTests
  set ::exitCode [expr {$numTests(Failed) >0}]
}

::tcltest::runAllTests

exit $exitCode

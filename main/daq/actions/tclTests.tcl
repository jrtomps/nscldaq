#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest

::tcltest::configure -testdir [file dirname [file normalize [info script]]]

#::tcltest::configure -outfile test.log

tcltest::testConstraint false 0
eval ::tcltest::configure $argv
::tcltest::runAllTests

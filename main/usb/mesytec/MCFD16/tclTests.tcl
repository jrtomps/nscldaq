#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
#
#  sshpipe.test is too finicky - can't get it to run quite right yet because
#  timing is not-deterministic.
#
::tcltest::configure -testdir [file dirname [file normalize [info script]]] \
    -notfile sshpipe.test
#::tcltest::configure -file *.test
tcltest::testConstraint false 0
eval ::tcltest::configure $argv


# add hook to access whether tests failed for generating a meaningful return code
proc ::tcltest::cleanupTestsHook {} {
  variable numTests
  parray numTests
  set ::exitCode [expr {$numTests(Failed) >0}]
}

#rename catch _real_catch
#set fail 0
#proc catch {script {ret ""} {opt ""}} {
#  set status 0
#  if {$ret ne ""} {
#    upvar $ret retvar
#    if {$opt ne ""} {
#      puts "$ret $opt"
#      upvar $opt optvar
#      set status [_real_catch $script retvar optvar]
#    } else {
#      puts "$ret"
#      set status [_real_catch $script retvar]
#    }
#  } else {
#    puts none
#    set status [_real_catch $script]
#  }
#
#  if {$status>0} {set ::fail $status}
#  return $status
#}
#
tcltest::runAllTests

exit $::exitCode

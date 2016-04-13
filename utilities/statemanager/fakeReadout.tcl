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
# @file fakeReadout.tcl
# @brief 
# @author Ron Fox <fox@nscl.msu.edu>
#

# This script provides just enough oomph to simulate a readout program.
# it's intended to be used to stand in for a real readout program in database
# statemachine run control testing.
#
#  It supports the following commands:
#
#  prebegin
#  begin
#  pause
#  resume
#  end
#  ring   (changes the output ringbuffer).
#

package provide dummyReadout 1.0

set run 0
set title {Set a new title}

proc prebegin {} {
    puts {run prebeginning}
    
}
proc begin {} {
    puts "Beginning a new run:"
    puts "Run    :   $::run"
    puts "Title  :   $::title"
}
proc pause {} {
    puts "Pausing the run"
    
}
proc prepause {} {
   puts "Prepausing"
}
proc preresume {} {
    puts "Preresuming"
}
proc resume {} {
    puts "Resuming the run"
}
proc preend {} {
    puts "Pre ending the run"
}
proc end {} {
    puts "Ending the run."
}
proc ring {name} {
    puts "Output ring buffer changed to $name"
    
}

package require StateManagerControl

vwait forever

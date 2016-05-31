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

package provide ReadoutCalloutsHarness 1.0


##
#  These global variables are maintained by the state manager:
#
set run 0
set title {Set a new title}


##
# Namespace for local/internal methods/procs

namespace eval ::RdoCallouts {
    ##
    # getRunNumber
    #    Return the current run number - this is an internal proc used to
    #    insulate us from the fact that the framework maintains the run number
    #    in a global variable:
    
    proc getRunNumber {} {
        return $::run
    }
}

##
# prebegin
#
#  pre-beginning a run.  If the proc
#  OnPrebegin exists it is called with the current run number as a parameter.
#
proc prebegin {} {
    if {[info procs OnPrebegin] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnPrebegin $run
    }
    
}
##
# begin
#   Beginning a new run.  If defined OnBegin is called with the run number
#   as a parameter.
#

proc begin {} {
    if {[info procs OnBegin] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnBegin $run
    }
}

##
# pause
#   Pausing an active run.  If defined, OnPause is called with the run number
#   as a parameter.
#
proc pause {} {
    if {[info procs OnPause] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnPause $run
    }    
}
##
# prepause
#    Entering the prepaused state - the run is about to be paused.
#    If defined, OnPrepause is invoked.  As usual instate transitions,
#    the run number is passed as a parameter.
#
proc prepause {} {
   if {[info procs OnPrepause] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnPrepause $run
    }   
}
##
# preresume
#    Entering the pre-resumed state.  A paused run is about to be resumed.
#    If defined, the OnPreresume proc is invoked with the run number
#    as a parameter.
#
proc preresume {} {
    if {[info procs OnPreresume] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnPreresume $run
    }   
}
##
# resume
#    The run is resuming.  If defined, OnResume is invoked with the run number
#    as a parameter.
proc resume {} {
    if {[info procs OnResume] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnResume $run
    }   
}
##
# preend
#    Entering t he preend state. An active or paused run is about to be ended.
#    If the OnPreend proc is defined it's called with the run number as a
#    parameter.
proc preend {} {
    if {[info procs OnPreend] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnPreend $run
    }   
}
##
# end
#   The run is ending.
#   If the OnEnd proc is defined, it's invoked with the run number as a parameter.
#
proc end {} {
    if {[info procs OnEnd] ne ""} {
        set run [RdoCallouts::getRunNumber]
        OnEnd $run
    }   
}
##
# ring
#    Set a new output ring name.
#  No-op.
#
proc ring {name} {
    
    
}

package require StateManagerControl

#-----------------------------------------------------------------------------
# The set of procs below emulate (as much as possible) the API provided by the
# old RedoutGUI:
#
#
#  NOTE:  We assume that the ::state::client command is a connection to the request
#         port of the database manager -- this is established by the
#         StatemanagerControl package... below we insulate from that
#         assumption so that putting an API in place is possible.
#

##
#  ::RdoCallouts::getReqApi
#
#    get the command that represents the API for making requests of the database
#    server.
#
# @return string - command name
#
proc ::RdoCallouts::getReqApi {} {
    return ::state::client
}

namespace eval ReadoutState {}

##
# setTitle
#   Ask the database manager to set a new title.
#   note that this will, in turn, result in a notification to us that the title
#   has changed which, in turn, will set our title global variable.
#
# @param string - new title string.
#
proc ReadoutState::setTitle {string} {
    set api [::RdoCallouts::getReqApi]
    
}
proc ReadoutState::getTitle {}
proc ReadoutState::setRun number
proc ReadoutState::getRun {}
proc ReadoutState::incRun {}
proc ReadoutState::setScalerCount scalers
proc ReadoutState::getScalerCount {}
proc ReadoutState::setScalerPeriod time
proc ReadoutState::getScalerPeriod {}
proc ReadoutState::getRecording {}
proc ReadoutState::enableRecording {}
proc ReadoutState::disableRecording {}
proc ReadoutState::setRecording {value}
proc ReadoutState::setRecording {value}
proc ReadoutState::isTimedRun
proc ReadoutState::TimedRun {}
proc ReadoutState::notTimedRun {}
proc ReadoutState::setTimedRun {state}
proc ReadoutState::timedLength {}
proc ReadoutState::setTimedLength {value}


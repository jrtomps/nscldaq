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
#-------------------------------------------------------------------------------
#
#   Simluation of the Readoutstate package:

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
    $api title $string
    
}
##
# getTitle
#   @return string - the current title.
proc ReadoutState::getTitle {} {
    set api [::RdoCallouts::getReqApi]
    return [$api title]
}
##
# setRun
#   Set a new run number value
#
# @param - integer new run number.
proc ReadoutState::setRun number {
    set api [::RdoCallouts::getReqApi]
    $api runNumber $number
}
##
# getRun
#  @return integer - current run number. 
proc ReadoutState::getRun {} {
    set api [::RdoCallouts::getReqApi]
    return [$api runNumber]
}
##
# incRun
#   Convenience function to increment the run number.
# @note - this is not atomic!!
proc ReadoutState::incRun {} {
    set r [::ReadoutState::getRun]
    incr i
    ::ReadoutStateSetRun $r
}
##
# setScalerCount
#   No op.
proc ReadoutState::setScalerCount scalers {}
# getScalerCount
#   Scaler counts are obsolete -- this just returns 0.
proc ReadoutState::getScalerCount {} {return 0}

##
# setScalerPeriod
#  - obsolete noop
proc ReadoutState::setScalerPeriod time {}
##
# getScalerPeriod
#   obsolete.
#
proc ReadoutState::getScalerPeriod {} {return 2}

##
# getRecording
#   @return bool - state of recording variable.
proc ReadoutState::getRecording {} {
    set api [::RdoCallouts::getReqApi]
    return [$api recording]
}
##
# enableRecording
#   Turn recording on for the next run.
#
proc ReadoutState::enableRecording {} {
    set api [::RdoCallouts::getReqApi]
    $api recording true
}
##
# disableRecording
#   Turn recording off, starting with the next run.
#
proc ReadoutState::disableRecording {} {
    set api [::RdoCallouts::getReqApi]
    $api recording false
}
##
# Set a new recording state.
#
# @param value - a Tcl bool for the new state
#
proc ReadoutState::setRecording {value} {
    set api [::RdoCallouts::getReqApi]
    if {$value} {
        $api recording true
    } else {
        $api recording false
    }
}

##
# isTimedRun
#   obsolete
# @return false
#
proc ReadoutState::isTimedRun {} {
    return false
}
##
# TimedRun - obsolete no op
proc ReadoutState::TimedRun {} {}
##
# notTimedRun
#   obsolete no op
proc ReadoutState::notTimedRun {} {}
##
# setTimedRun
#    obsolete no op
proc ReadoutState::setTimedRun {state} {}

##
# timedLength
#  Returns  0 since timed runs are obsolete.
#
proc ReadoutState::timedLength {} {return 0}
##
#  setTimedLength
#    obsolete so no op.
#
proc ReadoutState::setTimedLength {value} {}


#-------------------------------------------------------------------------------
#
#  Simulation of the ReadoutControl package

namespace eval ReadoutControl {}

##
# getReadoutState
#
#  @return string - the current state
#  @note The state returned is the system global state.  This means the values
#        you will get do not match those returned from the old getReadoutState.
#
proc ReadoutControl::getReadoutState {} {
    set api [::RdoCallouts::getReqApi]
    return [$api getGlobalState]
}
##
# isExecuting
#    @return boolean
#    @retval true - the state is not one of 0Initial, NotReady or Readying.
#
proc ReadoutControl::isExecuting {} {
     set state [::ReadoutControl::getReadoutState]
     
     return [expr {$state ni [list 0Initial NotReady Readying]}]
}
##
# SetRun
#     Set the current run number.
#
# @param n - new run number.
#
proc ReadoutControl::SetRun n {
    ReadoutState::setRun $n
}
##
# SetTitle
#    Set a new title.
#
# @param title - new title string.
proc ReadoutControl::SetTitle t {
    ReadoutState::setTitle $t 
}
##
# _transition
#   Internal method to perform a state transition
#
# @param s - desired state.
#
proc ReadoutControl::_transition s {
    set api [::RdoCallouts::getReqApi]
    $api   setGlobalState $s
    vwait  ::state::transitionCounter;    # Wait for my state to change.
    $api   waitTransition;                # Wait for all participants to complete.
}
##
# Begin
#    Begin a run.  Note that if the run is not in the  correct state to allow
#    Begin to work, an error will be signaled.
#
proc ReadoutControl::Begin {} {
    ::ReadoutControl::_transition Beginning
    ::ReadoutControl::_transition Active
}
##
# End
#   End the run. Note that if the run is not in a correct state to be ended,
#   an error will be signaled.
#
proc ReadoutControl::End {} {
    ::ReadoutControl::_transition Ending
    ::ReadoutControl::_transition Ready
}
##
# Pause
#    Pause and active run. An error is thrown if the run is not active.
#
proc ReadoutControl::Pause {} {
    ::ReadoutControl::_transition Pausing
    ::ReadoutControl::_transition Paused
}
##
# Resume
#   Resume a pause run.  An error is thrown if the run is not paused.
#
proc ReadoutControl::Resume {} {
    ::ReadoutControl::_transition Resuming
    ::ReadoutControl::_transition Active
}
#-----------------------------------------------------------------------------
#
#  Simulate the ReadoutGUI package.
#
namespace eval ReadoutGUI {}

##
# Start
#    Probably can't really do this but we're going to set the
#    system into Readying and then Ready mode.  The reason we probably can't do
#    this is that we are likely to be one of the programs that starts
#    up in readying.
#
proc ReadoutGUI::Start {} {
    ::ReadoutControl::_transition Readying
    ::ReadoutControl::_transition Ready
}
##
# Restart
#    -  Set the system to not ready.
#    -  Start
# @note - again this  probably can't really be done as if the program is
#         running uner the boot manager, It's going to exit when the
#         transition to not ready is posted.
#
proc ReadoutGUI::Restart {} {
    ::ReadoutControl::_transition NotReady
    ::ReadoutGUI::Start
}

##
# Begin
#   Start a run:
#
proc RedoutGui::Begin {} {
    ::ReadoutControl::Begin
}
##
# Pause
#   Pause an active run:
#
proc ReadoutGUI::Pause {} {
    ::ReadoutControl::Pause
}
##
# Resume
#   Resume a paused run:
#
proc ReadoutGUI::Resume {} {
    ::ReadoutControl::Resume
}
##
# StopRunTimers
#   - Obsolete - there are no run timers so no -op.
#
proc ReadoutGUI::StopRunTimers {} {}

##
# End
#   End an active or paused run.
#
proc ReadoutGUI::End {} {
    ::ReadoutControl::End
}
##
# SourceFile
#   Not implemented as there are multiple readouts it makes no sense to
#   source a Tcl file into a readout.


#------------------------------------------------------------------------------
#
#  Simulate a chunk of ReadoutGUIPanel.
#  Note that this namespace assumes Tk is installed.
#  Assumptions are that the GUI being affected is running in the toplevel
#  widget.
#
namespace eval ReadoutGUIPanel {}

##
# addUserMenu
#   Adds a user menu.  If a menubar does not yet exist for . it is created.
#   A new menu is added to the menubar with path derived from the
#   menubar and the ident. Its path is given back to the user.
#
# @param ident - used as the last path element for the menu.
# @param label - The menubutton's label.
# @return string - path to the menu so the user can stock it.
#
proc ReadoutGUIPanel::addUserMenu  {ident label} {
    package require Tk
    # If necessary, create the menu.
    
    if {[. cget -menu] eq ""} {
        
        # Probe for an unused widget name:
        
        set index 0
        while {".m_$index" ni [winfo children .]} {
            incr index
        }
        menu .m_$index
        . configure -menu .m_$index
    }
    #  Get the top level's menu:
    #  Create the submenu and attach it to a new cascade:
    
    set menubar [. cget -menu]
    set menu [menu $menubar.$ident]
    $menubar add cascade -menu $menu -label $label
    
    return $menu
}
##
#  addUserFrame
#     Add a frame to the toplevel widget.  The frame widget path is passed
#     back to the user.  In this implementation grid is used to layout
#     the widgets.  The frame is placed below all existing frames and
#     setup to be sticky on all sides to its bounding box.
#
# ident   - identifies the frame.  This will be the last element of the widget path.
# @return string - frame's full widget path.
proc ReadoutGUIPanel::addUserFrame {ident} {
    package require Tk
    
    set w [ttk::frame .$ident]
    grid $w -sticky nsew
    return $w
    
}
##
# getHost
#   Unimplemented as the readout system is distributed across a pile of hosts
#   (potentially),.
#

##
# getPath
#   Unimplemented as there can be several readout programs.
#


##
# setTitle
#    Set the UI title - the best way to do this is to set the actual title.
#
# @param title - new title string
#

proc ReadoutGUIPanel::setTitle title {
    ReadoutState::setTitle $title
}
##
# getTitle
#   Get the value of the UI title - this will be the same as the title in the
#   database so:
#
# @return string - the current title.
#
proc ReadoutGUIPanel::getTitle {} {
    return [ReadoutState::getTitle]
}
##
# getRunNumber
#   Gets the run number from the GUI - this will be the same as the run number in
#   the database so:
#
# @return int - the run number.
#
proc ReadoutGUIPanel::getRunNumber {} {
    return [ReadoutState::getRun]
}
##
# setRun
#   Set the run number in the GUI - best way to do that is to set it in the
#   database.
#
# @param run - new run number.
#
proc ReadoutGUIPanel::setRun run {
    ReadoutState::setRun $run
}
##
# incrRun
#   Increment the run number in the GUI/database.
#
proc ReadoutGUIPanel::incrRun {} {
    ReadoutState::incRun
}
##
# setHost - obsolete

##
# setPath - obsolete

##
# recordOff
#   Disable event recording in the ui by disabling it in the database.
#
proc ReadoutGUIPanel::recordOff {} {
    ReadoutState::disableRecording
}
##
# recordOn
#   Enable event recording in the UI by enabling it in the database.
#
proc ReadoutGUIPanel::recordOn {} {
    ReadoutState::enableRecording
}
##
# setScalers - obsolete unimplemented.

##
# getScalers - obsolete unimplemented.


##
# recordData
#  This badly named proc returns the state of the recording flag.
#
# @return bool - true if recording is enabled.
#
proc ReadoutGUIPanel::recordData {} {
    return [ReadoutState::getRecording]
}

##
# outputText
#    Outputs text to stdout.  In the original implementation, this output text to
#    a UI text window. In this implementationthat can only happen if the application
#    has started in a TkCon - which will capture stdout, stderr and displa it
#    in a scrolling text widget.
#
# @param text   - the text to output.
#
proc ReadoutGUIPanel::outputText text {
    puts $text
}
##
# isTimed
#   Obsolete - returns false always as there are no timed runs.
#
proc ReadoutGUIPanel::isTimed {} {
    return false
}
##
# setTimed - obsolete/unimplemented.

##
# getRequestedRunTime obsolete/unimplemented

##
# setRequestedRunTime - obsolete/unimplemented.







package require StateManagerControl
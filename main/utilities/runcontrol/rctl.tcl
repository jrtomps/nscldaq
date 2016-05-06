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
# @file rctl.tcl
# @brief Simple command line run control script for State manager RCTL.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQROOT] eq "DAQROOT"} {
    lappend auto_path [file join $::env(DAQROOT) TclLibs]
}
package require stateclient


#  Use environment variables to figure out the server:

namespace eval ::runcontrol {
    variable requri        $::env(REQ_URI)
    variable suburi        $::env(SUB_URI)
    variable programFailed 0
    variable exiting       0
    
    variable commands [list begin end pause resume kill exit]
}


#  Note that this script only does global state transitions.

##
# prompt
#   output the command prompt.
#
#
proc prompt {} {
    puts -nonewline "RunControl> "
    flush stdout
}


# reportStateChange
#
#  Report state change in a program during global state change
#
# @param program - name of the program that changed state.
# @param state   - That program's new state.
#
proc reportStateChange {program state} {
    puts "$program changed state to $state"
}

# reportEvent
#
#  Report something happened to the world.  Note that a program
#  state change to NotReady will flag that we need to do a global
#  state change to not ready.
#
# @param infoDict - dict that describes the event.
#
proc reportEvent {infoDict} {
    set reason [dict get $infoDict type]
    if {$reason eq "GlobalStateChange"} {
        puts "Some other program changed the global state to [dict get $infoDict state]"
    } elseif {$reason eq "ProgramStateChange"} {
        set program [dict get $infoDict program]
        set state   [dict get $infoDict state]
        puts "$program  changed state to $state"
        
        if {$state eq "NotReady"} {
            puts "Bringing down the system due to program crash"
            ::runcontrol::client setGlobalState NotReady
            ::runcontrol::client waitStateTransition reportStateChange
        }
        
    } elseif {$reason eq "ProgramJoins"} {
        puts "A new program joined the system: [dict get $infoDict program]"
        
    } elseif {$reason eq "ProgramLeaves"} {
        puts "An existing program left the system: [dict get $infoDict program]"
    }
}

##
# checkEvents
#   periodically flush the event queue when we're not doing a state transition:
#
proc checkEvents {} {
    after 500 checkEvents
    
    ::runcontrol::client processMessages reportEvent
}


##
# processCommand
#   stdin is readable.  Read the command.  If it's a legitimate command,
#   execute it.  Regardless prompt for the next command.
#
proc processCommand {} {
    set command [gets stdin]
    if {$command ni $::runcontrol::commands} {
        puts "Illegal command: $command.  Must be one of : $::runcontrol::commands"
    } else {
        set newstate ""
        if {$command eq "begin"} {
            set newstate Beginning
        } elseif {$command eq "end" } {
            set newstate Ending
        } elseif {$command eq "pause"} {
            set newstate Pausing
        } elseif {$command eq "resume"} {
            set newstate Resuming
        } elseif {$command eq "kill"} {
            set newstate NotReady
        } elseif {$command eq "exit"} {
            incr ::runcontrol::exiting
        }
        
        # Execute state transition if appropriate:
        
        if {$newstate ne ""} {
            ::runcontrol::client setGlobalState $newstate
            if {! [::runcontrol::client waitTransition reportStateChange]} {
                puts "Global state transition timed out"
            }
        }
    }
    prompt
}

#  Connect to the state manager:

::nscldaq::statemanager  ::runcontrol::client $::runcontrol::requri $::runcontrol::suburi

fileevent stdin readable processCommand
checkEvents

prompt

vwait ::runcontrol::exiting
puts "Exiting"
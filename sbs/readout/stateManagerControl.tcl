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
# @file stateManagerControl.tcl
# @brief Package to control Readout using the state manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide StateManagerControl 1.0

# If DAQROOT is a defined environment variable, then we can
# do this, otherwise we have to take our chances on TCLIBPATH or some other
# extension to the library search path having been done.

if {[array names env DAQROOT] eq "DAQROOT"} {
    lappend auto_path [file join $::env(DAQROOT) TclLibs]
}
package require stateclient

# need to have the following ENV variables defined (usually by the boot manager):
#
#  REQ_URI - URI of the server's request port.
#  SUB_URI - URI of the server's subscription port.l
#  PROGRAM - Program name.

namespace eval state {
    variable reqUri      $::env(REQ_URI)
    variable subUri      $::env(SUB_URI)
    variable programName $::env(PROGRAM)
    variable runActive    0
    variable handledStates [list NotReady Beginning Ending Pausing Resuming]

}

#  Connect to the state manager:

::nscldaq::stateclient ::state::client \
    $::state::reqUri $::state::subUri $::state::programName


#  When we start the global state had better be "Readying"
#  We'll try to set our state to "Ready" and if that fails,
#  We must fail too:

if {[::state::client getstate] ne "Readying"} {
    error "Starting a readout but the state is not 'Readying'"
    exit 1
}

if {[catch {::state::client setstate Ready} msg]} {
    error "Unable to set program state to 'Ready': $msg"
    exit 1
}


# enable callback to andle state transitions:

::state::client onStateChange stateChanged

##
#  Do a pre-begin or die trying .  This puts us in the 'Starting' state.
#
proc prebeginOrDie {} {
    
    # Let's update the title and run numbers...these might fail if the run is
    # active, in which case the begin will also fail:
    
    catch {set ::run [::state::client runnumber]}
    catch {set ::title [::state::client title]}
    set outring [::state::client outring]
    if {$outring ne ""} {
        catch {ring $outring}
    }
    if {[catch prebegin msg]} {
        ::state::client setstate NotReady
        error "Failed to perform pre-begin operations: $msg"
        exit -1
    }
    
}
proc startRunOrDie {} {


    
    if {[catch begin msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to begin a run: $msg"
        exit -1
    }    
    set ::state::runActive 1
}

proc pauseRunOrDie {} {
    if {[catch pause msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to pause a run: $msg"
        exit -1
    }
    set ::state::runActive 1 ;             # Should already be.
    
}

proc resumeOrDie {} {
    if {[catch resume msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to resume a run: $msg"
        exit -1
    }
    set ::state::runActive 1;                # Should already be
    ::state::client setstate Active
}

proc endOrDie {} {
    if {[catch end msg]} {
        ::state::client setstate NotReady
        error "Failed to end the run: $msg"
        exit -1
    }
    set ::state::runActive 0
    ::state::client setstate Ready
}

##
# handleStandaloneStateTransition
#   Handle a standalone state transition.
#
# @param newState - the new local state.
#
proc handleStandaloneStateTransition newState {
    #
    # Handle all the externally stimulated transitions. Note that we'll get
    # notified of the internally stimulated transitions so we're going to ignore those.
    #
    
    #  Being told to exit: 
    if {$newState eq "NotReady"} {
        if {$::state::runActive} {
            end
        }
        exit 0
        
        #  Beginning a run - key point don't echo Beginning - that's not legal.
    } elseif {$newState eq "Beginning"} {
        prebeginOrDie
        
    } elseif {$newState eq "Active"} {
        beginRunOrDie
        
        # Pausing a run:
    } elseif {$newState eq "Pausing"} {
        pauseRunOrDie
        
        # Resuming a run:
    } elseif {$newState eq "Resuming"} {
        resumeOrDie
        
        # Ending a run:
    } elseif {$newState eq "Ending"} {
        endOrDie
    }
    # Echo the new state in our state:
    
}
##
# handleGlobalStateTransition
#   Handle state transitions that are global.
#
# @param newState - the new global state.
#
proc handleGlobalStateTransition newState {
    #
    #  only nandle the state transitions we support.
    #  
    
    if {$newState in $::state::handledStates} {
        handleStandaloneStateTransition $newState
    }
    ::state::client setstate $newState;     # Must echo new state as local state.
}

##
#  stateChanged
#    Process state changes.
#    What we do depends a lot on whether we are enabled, standalone or not.
#
proc stateChanged newState {
    #
    #  If disabled all state transitions get ignored.
    #
    if {[::state::client isenabled]} {
        # Handling transitions differs a bit from standalone/not standalone mode:
        
        if {[::state::client isstandalone]} {
            handleStandaloneStateTransition $newState
        } else {
            handleGlobalStateTransition $newState
        }
            
    }
}

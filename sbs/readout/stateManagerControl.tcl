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
    variable handledStates [list NotReady Beginning Active Ending Ready Pausing Resuming]
    variable lastGlobalState Readying

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
proc beginRunOrDie {} {


    
    if {[catch begin msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to begin a run: $msg"
        exit -1
    }    
    set ::state::runActive 1

}
proc prePauseRunOrDie {} {
    if {[catch prepause msg]} {
        ::state::client setState NotReady
        end
        error "Failed to prepause a run: $msg"
    }
    set ::state::runActive 1;    # Should aready be
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
proc preResumeOrDie {} {
    if {[catch preresume msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to pre resume a run"
    }
    set ::state::runActive 1;             # Should aready be active.
}
proc resumeOrDie {} {
    if {[catch resume msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to resume a run: $msg"
        exit -1
    }
    set ::state::runActive 1;                # Should already be
    
}
proc preEndOrDie {} {
    if {[catch preend msg]} {
        ::state::client setstate NotReady
        end
        error "Failed to end a run: $msg"
        exit -1
    }
    set ::state::runActive 1;               # In the middle of a run until 'end'.
}
proc endOrDie {} {
    if {[catch end msg]} {
        ::state::client setstate NotReady
        error "Failed to end the run: $msg"
        exit -1
    }
    set ::state::runActive 0
    
}

##
# handleStandaloneStateTransition
#   Handle a standalone state transition.
#
# @param newState - the new local state.
# @return nextState - Next the program state should be set to if this is a
#                     global state transition.
#
proc handleStandaloneStateTransition  newState {
    #
    # Handle all the externally stimulated transitions. Note that we'll get
    # notified of the internally stimulated transitions so we're going to ignore those.
    #
    
    #  Being told to exit: 
    if {$newState eq "NotReady"} {
        if {$::state::runActive} {
            end
            ::state::client setstate NotReady
            exit
        }
        exit 0
        
        #  Beginning a run - key point don't echo Beginning - that's not legal.
    } elseif {$newState eq "Beginning"} {
        prebeginOrDie
        return "Beginning"
        
    } elseif {$newState eq "Active"} {
        beginRunOrDie
        return "Active"
        
        # Pausing a run:
    } elseif {$newState eq "Pausing"} {
        prePauseRunOrDie
        return Pausing
    } elseif {$newState eq "Paused"} {
        
        pauseRunOrDie
        return Paused
        
        # Resuming a run:
    } elseif {$newState eq "Resuming"} {
        preResumeOrDie
        return Resuming
    } elseif {$newState eq "Resume"} {
        resumeOrDie
        return Active
        # Ending a run:
    } elseif {$newState eq "Ending"} {
        preEndOrDie
        return Ending
        
    } elseif {$newState eq "Ready"} {
        if {$::state::runActive} {
            endOrDie
            return Ready
        }
    }

    
}
##
# dispatchStateTransition
#
#   Called when we have a global state transition we do want to handle.
#   What we do may depend on the prior state as well as the requested state.
#
#   If action is required, we will pass that on to handleStandaloneStateTransition
#   otherwise the 'ing state has already set the correct final state.
#
# @param from - Prior global state.
# @param to   - New global state.
# @return next desired state of program. "" means don't set the state.
#
proc dispatchStateTransition {from to} {
    
    # At present the only thing that's affected is a transition to the Active
    # state.... If we got here from Resuming, we do nothing... as Resume
    # will set us active already:
    
    if {$to eq "Active"} {
        if {$from eq "Beginning"} {
            return [handleStandaloneStateTransition $to]
        } else {
            return ""
        }
    } elseif {$to eq "Ready"} {
        if {$from eq "Ending"} {
            return [handleStandaloneStateTransition $to]
        } else {
            return ""
        }
    } else {
        #  All other transitions are ok to pass on unconditionally -- for now.
        
        return [handleStandaloneStateTransition $to]
    }

    
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
        set oldNextState $newState
            
        
        set newState [dispatchStateTransition \
            $::state::lastGlobalState $newState                 \
        ]
        if {$newState ne ""} {
            if {[catch {::state::client setstate $newState} msg]} {
                set newState $oldNextState
            } 
        }
    }
    set state::lastGlobalState $newState;     # Need to know come from.
    
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

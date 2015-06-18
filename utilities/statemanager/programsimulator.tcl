

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   programsimulator.tcl
# @brief  Functions to simulate programs.
# @author <fox@nscl.msu.edu>


# This file contains procs that can be used from inside a vardbsh
# talking to a server to simulate various state transtions.
# The programs in the system are assumed to be in the list:
#  ::programs

set programs [list test aname];  # Modify this if required.

#
# fail the system

proc fail {} {
    catch {transitionPrograms NotReady}
    catch {let /RunState/State NotReady}
}

# Requries everyone to be 0Initial
# takes us to Ready:
proc readySystem {} {
    fail
    
    transitionPrograms Readying
    transitionPrograms Ready
    
    let /RunState/State Readying
    let /RunState/State Ready
}

#  Simluate a begin run:

proc  beginRun {} {
    transitionPrograms Beginning
    transitionPrograms Active
}
# Pause a run

proc pauseRun {} {
    transitionPrograms Pausing
    transitionPrograms Paused
}

proc endRun {} {
    transitionPrograms Ending
    transitionPrograms Ready
}

proc resumeRun {} {
    transitionPrograms Resuming
    transitionPrograms Active
}

#  Make all programs take a state transition.

proc transitionPrograms newState  {
    foreach program $::programs {
        let /RunState/$program/State $newState
    }
}


#!/bin/sh

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
# @file MakeRunControl.tcl
# @brief Create top level run control directory.
# @author Ron Fox <fox@nscl.msu.edu>
#

# For what this does see the "Variable Database Run Control"
# wiki page in the NSCLDAQ redmine.

#
#  Runs a command ignoring the errors from it
#  but reporting them.
#
proc ignoreError args {
    set status [catch {
        uplevel  $args    
    } msg]
    if {$status} {
        puts $args
        puts $msg
    }
}

# Create the directory and define the state machine:


ignoreError mkdir /RunState
ignoreError statemachine RunStateMachine [dict create                       \
    0Initial NotReady                                                       \
    NotReady    [list 0Initial Readying]                                    \
    Readying    [list NotReady Ready]                                       \
    Ready       [list Beginning NotReady]                                   \
    Beginning   [list Active NotReady]                                      \
    Active      [list Pausing Ending NotReady]                              \
    Pausing     [list Paused NotReady]                                      \
    Paused      [list Ending Resuming NotReady]                             \
    Resuming    [list Active NotReady]                                      \
    Ending      [list Ready NotReady]                                       \
]
ignoreError enum boolean [list true false]


# Now create the variables in /RunState:

set wd [pwd];                                 # Want to restore wd later.

cd /RunState
ignoreError declare RunNumber           integer
ignoreError declare Title               string
ignoreError declare Timeout             integer
ignoreError declare ReadoutParentDir    string

# Some items neeed to be set to some reasonable initial values:

# The state will be set to 0initial.  The two lets will work regardless of
# the initial state.

ignoreError declare State               RunStateMachine
let State NotReady
let State 0Initial

# Initialize to not recording data.

ignoreError declare Recording           boolean
let Recording       false


cd $wd;                                      # Restore wd.


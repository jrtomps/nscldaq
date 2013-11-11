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
# @file ReadoutState.tcl
# @brief Maintain run state
# @author Ron Fox <fox@nscl.msu.edu>

package provide ReadoutState 2.0
package require Configuration
package require RunstateMachine

namespace eval  ReadoutState {
    namespace export attach enter leave
}

# ReadoutState::setDefaults
#    Set the default values of the configuration items.
#
proc ReadoutState::setDefaults {} {
    Configuration::Set RunTitle       {Set a new title}
    Configuration::Set RunNumber      1
    Configuration::Set Recording      0;       # Event recording enabled.
    Configuration::Set Timed          0;       # Timed run enabled.
    Configuration::Set TimedLength    0;       # Seconds of a timed run.
}

# ReadoutState::environmentOverrides
# Override any of the overridable members via environment variables:
#    Configuration            EnvName:
#    RunTitle                 EXPTITLE
#
proc ReadoutState::environmentOverrides {} {

   Configuration::readEnvironment RunTitle        EXPTITLE {Set a new title}

}
# ReadoutState::setTitle string
#     Sets a new value for the title configuration item.  This is the title
#     that is programmed into the Readout program when a run starts.
# Parameters:
#     string   -new title string.
#
proc ReadoutState::setTitle {string} {
    Configuration::Set RunTitle  $string
}
# ReadoutState::getTitle
#    Returns the current value of the title string.
#
proc ReadoutState::getTitle {} {
    return [Configuration::get RunTitle]
}
# ReadoutState::setRun   number
#    Set the current run number.
# Parameters:
#   number   - The new run number.
# Errors:
#   NotInteger   - The number parameter must be an integer and is not.
#   Negative     - The run number must be >= 0.
#
proc ReadoutState::setRun number {
    if {![string is integer -strict $number]} {
        error "ReadoutState::NotInteger run number value was: $number"
    }
    if {$number < 0} {
        error "ReadoutState::Negative Run number must be an integer greater than zero was: $number"
    }

    Configuration::Set RunNumber $number
}
# ReadoutState::getRun
#    Returns the current run number.
#
proc ReadoutState::getRun {} {
    return [Configuration::get RunNumber]
}
# ReadoutState::incRun
#    Increment the run number.  This is a common
#   enough occurence in daily running that we support
#   it here.
#
proc ReadoutState::incRun {} {
    set run [Configuration::get RunNumber]
    incr run
    Configuration::Set RunNumber $run
}
# ReadoutState::setScalerCount scalers
#     Set the number of scalers in the configuration.
# Parameters:
#     scalers - Number of scalers to tell the Readout software
#               to use.
# Errors:
#     NotInteger  - scalers is not an integer.
#     Negative    - scalesr is < 0.
proc ReadoutState::setScalerCount scalers {
    if {![string is integer -strict $scalers]} {
        error "ReadoutState::NotInteger - Scaler count was not an integer was: $scalers"
    }
    if {$scalers < 0} {
        error "ReadoutState::Negative - Scaler count was negative: $scalers"
    }
    Configuration::Set ScalerCount $scalers
}
# ReadoutState::getScalerCount
#    Get the number of scalers in the configuration.
#
proc ReadoutState::getScalerCount {} {
    return [Configuration::get ScalerCount]
}
# ReadoutState::getRecording
#     Return state of the recording flag.
#
proc ReadoutState::getRecording {} {
    return [Configuration::get Recording]
}
#
#   Set recording state to various values.
#
proc ReadoutState::enableRecording {} {
    Configuration::Set Recording 1
}
proc ReadoutState::disableRecording {} {
    Configuration::Set Recording 0
}
proc ReadoutState::setRecording {value} {
    Configuration::Set Recording $value
}
# ReadoutState::isTimedRun
# ReadoutState::TimedRun
# ReadoutState::notTimedRun
# ReadoutState::setTimedRun
#
#     Get and set state of timed run configuration.
#

proc ReadoutState::isTimedRun {} {
    Configuration::get Timed
}

proc ReadoutState::TimedRun {} {
    Configuration::Set Timed 1
}
proc ReadoutState::notTimedRun {} {
    Configuration::Set Timed 0
}
proc ReadoutState::setTimedRun {state} {
    Configuration::Set Timed $state
}
# ReadoutState::timedLength
#     Return the timed run length.
#
proc ReadoutState::timedLength {} {
    Configuration::get TimedLength
}
# ReadoutState::setTimedLength value
#     Set the timed run length.
#
proc ReadoutState::setTimedLength {value} {
    Configuration::Set TimedLength $value
}

#------------------------------------------------------------------------------
#
#   Bundle interfaces - These methods allow the package to be registered
#                       in the run state machine.  Mostly this is just to
#                       maintain the run number when recording is enabled.
#
##
# ::ReadoutState::attach
#
#   Invoked when the callout bundle is first registered to the state machine.
#   For now this does nothing.
#
proc ::ReadoutState::attach {state} {}
##
# enter
#   Called when a new state is entered (after the state transition is complete)
#   This increments the run number if:
#   * The prior state was one of [list Active Paused]
#   * Recording was enabled for the run.
#
# @param from - Prior state.
# @param to   - Next state.
#
proc ::ReadoutState::enter {from to} {
    if {$from in [list Active Paused]} {
        if {[::ReadoutState::getRecording]} {
            ::ReadoutState::incRun
        }
    }
}
##
# leave
#
#   Called when a state is being left (state variable not  yet changed)/
#
# @param from - Prior state.
# @param to   - Next state.
#
proc ::ReadoutState::leave {from to} {}


#------------------------------------------------------------------------------
#  Registration/unregistration
#

##
# register
#
#   Register the bundle with the readout state machine singleton.
#
proc ::ReadoutState::register {} {
    set stateMachine [RunstateMachineSingleton  %AUTO%]
    $stateMachine addCalloutBundle ReadoutState
    $stateMachine destroy
}
##
# unregister
#
#   Provided for testing...unregisters the bundle from the state machine singleton
#
proc ::ReadoutState::unregister {} {
    set stateMachine [RunstateMachineSingleton  %AUTO%]
    $stateMachine removeCalloutBundle ReadoutState
    $stateMachine destroy
    
}


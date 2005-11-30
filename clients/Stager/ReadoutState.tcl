#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  RunState - This package manages the configuration and state of data taking
#             (readout initial parameters).  Its purpose is to provide persistent
#             state from run to run of the readout program/gui/stager.
#
package provide ReadoutState 1.0
package require Configuration

namespace eval  ReadoutState {
}

# ReadoutState::setDefaults
#    Set the default values of the configuration items.
#
proc ReadoutState::setDefaults {} {
    Configuration::Set RunTitle       {Set a new title}
    Configuration::Set RunNumber      1
    Configuration::Set ScalerCount    0
    Configuration::Set ScalerInterval 2;       # more common than the old 10.
    Configuration::Set Recording      0;       # Event recording enabled.
    Configuration::Set Timed          0;       # Timed run enabled.
    Configuration::Set TimedLength    0;       # Seconds of a timed run.
}

# ReadoutState::environmentOverrides
# Override any of the overridable members via environment variables:
#    Configuration            EnvName:
#    RunTitle                 EXPTITLE
#    ScalerCount              SCALERS
#    ScalerInterval           SCALERINTERVAL
#
proc ReadoutState::environmentOverrides {} {

   Configuration::readEnvironment RunTitle        EXPTITLE {Set a new title}
   Configuration::readEnvironment ScalerCount     SCALERS        0
   Configuration::readEnvironment ScalerInterval  SCALERINTERVAL 2

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
        error ReadoutState::NotInteger
    }
    if {$number < 0} {
        error ReadoutState::Negative
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
        error ReadoutState::NotInteger
    }
    if {$scalers < 0} {
        error ReadoutState::Negative
    }
    Configuration::Set ScalerCount $scalers
}
# ReadoutState::getScalerCount
#    Get the number of scalers in the configuration.
#
proc ReadoutState::getScalerCount {} {
    return [Configuration::get ScalerCount]
}
# ReadoutState::setScalerPeriod time
#
#    sets the number of seconds between scaler readouts. This
#    must be an integer value > 0.
# Parameters:
#    time   - Number of seconds desired between scaler readouts.
# Errors:
#    NotInteger - time is not an integer.
#    LeZero     - Time is <= 0.
#
proc ReadoutState::setScalerPeriod time {
    if {![string is integer -strict $time]} {
        error ReadoutState::NotInteger
    }
    if {$time <= 0} {
        error ReadoutState::LeZero
    }
    Configuration::Set ScalerInterval $time
}
# ReadoutState::getScalerPeriod
#    Return the number of seconds between periodic scaler events.
#
proc ReadoutState::getScalerPeriod {} {
    return [Configuration::get ScalerInterval]
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

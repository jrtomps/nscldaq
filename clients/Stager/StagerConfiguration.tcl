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

# This file manages the configuration of the stager software.  The
# stager software is responsible for swooshing data out to tape from
# the stage area.  It knows about the following configuration variables:
#   Name         Env       Default    Means
# TapeHost     TAPEHOST     --        The host that has the tapedrive.
# TapeCapacity TAPESIZE    100Gbytes  Size of a tape cartridge used.
# TapeNumber     --        1          Tape serial number.
# StagePolicy  STAGEPOLICY StagePolicyFree When data are staged.
# StageThreshold --        0.75       Fraction of 'full' that triggers a stage
# TapeDrive     TAPE       --         Dev special with tapedrive.
# StageList     --         --         set of runs that have been staged.
# RetainedList  --         --         Set of runs that have been retaained after stage.
#
package provide StagerConfiguration 1.0
namespace eval StagerConfiguration {
}
package require Configuration

# StagerConfiguration::isOkInteger value ?limit?
#     If the value is not an 'ok' integer, an error is thrown.
#     This factors out this sort of checking in quite a few
#     procs below.
# Parameters:
#   value    - The value to check.
#   limit    - If provided the lower limit on the value.
#              If omitted, 0.
# Errors:
#   NotInteger   - Tape capacity is not an integer.
#   TooSmall     - capacity < 1Gbyte (even a mem stick can be that big).
#
proc StagerConfiguration::isOkInteger {value {limit 0}} {
    if {![string is integer -strict $value]} {
        error StagerConfiguration::NotInteger
    }
    if {$value < $limit} {
        error StagerConfiguration::TooSmall
    }

}


#
#   StagerConfiguration::setDefaults
#      Sets the default values of the configuration parameters to their
#      defaults.
#
proc StagerConfiguration::setDefaults {} {
    Configuration::Set TapeCapacity    100
    Configuration::Set TapeNumber        1
    Configuration::Set StagePolicy    StagePolicyFree
    Configuration::Set StageThreshold 0.75
    Configuration::Set StageList       ""
    Configuration::Set RetainedList    ""
}
# StagerConfiguration::environmentOverrides
#    Applies defined environment variables to the default
#  configuration.
#
proc StagerConfiguration::environmentOverrides {} {
    Configuration::readEnvironment TapeHost     TAPEHOST
    Configuration::readEnvironment TapeCapacity TAPESIZE    100
    Configuration::readEnvironment StagePolicy  STAGEPOLICY StagePolicyFree
    Configuration::readEnvironment TapeDrive    TAPE
}
# StagerConfiguration::getTapeHost
#    Return the currently configured tape drive host.
#
proc StagerConfiguration::getTapeHost {} {
    return [Configuration::get TapeHost]
}
# StagerConfiguration::tapeHostIs host
#    Set the new name of the tape drive host.
#    The tape drive host must accept connections for the
#    shell in order for tar to access remote drives.
# Parameters:
#    host
# Errors:
#    NoRshServer  - The host, if it exists is not running a
#                   shell server.
#
proc StagerConfiguration::tapeHostIs host {
    if {[catch [list socket $host shell] sock]} {
        error StagerConfiguration::NoRshServer
    } else {
        close $sock
        Configuration::Set TapeHost $host
    }
}
# StagerConfiguration::getTapeCapacity
#    Return the currently configured tape capacity (in Gbytes).
#
proc StagerConfiguration::getTapeCapacity {} {
    return [Configuration::get TapeCapacity]
}
# StagerConfiguration::tapeCapacityIs capacity
#    Sets a new value for the tape capacity in Gbytes.
# Parmeters:
#   capacity  - New capacity.
# Errors:
#   NotInteger   - Tape capacity is not an integer.
#   TooSmall     - capacity < 1Gbyte (even a mem stick can be that big).
#
proc StagerConfiguration::tapeCapacityIs capacity {
    isOkInteger $capacity 1
    Configuration::Set TapeCapacity $capacity

}
# StagerConfiguration::getTapeNumber
#    Returns the tape number from the configuration database.
#
proc StagerConfiguration::getTapeNumber {} {
    return [Configuration::get TapeNumber]
}
# StagerConfiguration::tapeNumberIs num
#   Set a new tape serial number.
# Parameters:
#   num - The new number.
# Errors:
#  NotInteger   - The tape number is not an integer.
#  TooSmall     - The tape number is < 1.
#
proc StagerConfiguration::tapeNumberIs num {
    isOkInteger $num 1

    Configuration::Set TapeNumber $num
}
# StagerConfiguration::incrTapeNumber
#    Increment the tape number.
#
proc StagerConfiguration::incrTapeNumber {} {
    set n [Configuration::get TapeNumber 0]
    incr n
    Configuration::Set TapeNumber $n

}
# StagerConfiguration::stagePolicyIsFree
#    Sets the stage policy to one that looks at the free
#  space on the media.
#
proc StagerConfiguration::stagePolicyIsFree {} {
    Configuration::Set StagePolicy StagePolicyFree
}
# StagerConfiguration::stagePolicyIsUsed
#   Set the stage policy to one that monitors the space used
#   by this experiment
proc StagerConfiguration::stagePolicyIsUsed {} {
    Configuration::Set StagePolicy StagePolicyUsed
}
# StagerConfiguration::getStagePolicy
#   Returns the current stage policy.
#
proc StagerConfiguration::getStagePolicy {} {
    return [Configuration::get StagePolicy]
}
# StagerConfiguration::getStageThreshold
#    Return the current stage policy threshold value.
#
proc StagerConfiguration::getStageThreshold {} {
    return [Configuration::get StageThreshold]
}
# StagerConfiguration::stageThresholdIs value
#    Sets the staging trigger threshold to a new value.
# Parameters:
#  value  - the new threshold.  Note that this must be
#           a floating point in the range (0.1, 0.9)
# Errors:
#   NotFloat   - value is not a floating point number.
#   TooSmall   - Value is <= 0.1
#   TooBig     - Value is >= 0.9 (trying to give some leeway).
#
proc StagerConfiguration::stageThresholdIs value {
    if {![string is double -strict $value]} {
        error StagerConfiguration::NotFloat
    }
    if {$value <= 0.1} {
        error StagerConfiguration::TooSmall
    }
    if {$value >= 0.9} {
        error StagerConfiguration::TooBig
    }
    Configuration::Set StageThreshold $value
}
# StagerConfiguration::getTapeDrive
#    Returns the tape device name.
#
proc StagerConfiguration::getTapeDrive {} {
    return [Configuration::get TapeDrive]
}
# StagerConfiguration::tapeDriveIs drive
#    Set a new value for the tape device name.
#
# Parameters:
#   drive  - the tape drive.
#
proc StagerConfiguration::tapeDriveIs drive {
    Configuration::Set TapeDrive $drive
}
# StagerConfiguration::getStageList
#    Returns the list of runs that have been staged out.
#
proc StagerConfiguration::getStageList {} {
    return [Configuration::get StageList]
}
# StagerConfiguration::appendStageList run
#    Appends a new run number to the staged runs list.
# Parameters:
#   run   - Run number to append.
# Errors:
#   NotInteger  - run must be an integer.
#   TooSmall    - Run must be >= 0.
#
proc StagerConfiguration::appendStageList run {
    isOkInteger $run

    set staged [Configuration::get StageList]
    lappend staged $run
    Configuration::Set StageList $staged
}
# StagerConfiguration::clearStageList
#    Clear the staged runs file.
#
proc StagerConfiguration::clearStageList {} {
    Configuration::Set StageList ""
}
# StagerConfiguration::setStageList intlist
#   Replaces the current staged run list with intlist.
# Parameters:
#   intlist - the new list of runs
# Errors:
#    InvalidParameter - One of the runs does not satisfy the
#                       conditions of appendStageList.
#
proc StagerConfiguration::setStageList intlist {
    set oldList [Configuration::get StageList]

    StagerConfiguration::clearStageList
    foreach run $intlist {
        if {[catch {StagerConfiguration::appendStageList $run}]} {
            if {[llength $oldList] > 0} {
                StagerConfiguration::setStageList $oldList; # Presumably this is an ok list.
            }
            error StagerConfiguration::InvalidParameter
        }
    }
}
# StagerConfiguration:appendRetainList run
#     Appends a run to the retain list.
# Parameter:
#    run   - The new run-number.
# Errors:
#   NotInteger  - run must be an integer.
#   TooSmall    - Run must be >= 0.
#
proc StagerConfiguration::appendRetainList {num} {
    isOkInteger $num
    set rlist [Configuration::get RetainedList]
    lappend rlist $num
    Configuration::Set RetainedList $rlist
}

# StagerConfiguration::setRetainList list
#       Provides a new value for the list of retained runs.
# Parameters:
#    intlist    - New list of runs.
# Errors:
#   InvalidParameter - One of the runs does not satisfy the
#                      conditions of appendRetainList
#
proc StagerConfiguration::setRetainList values {
    set oldList [StagerConfiguration::getRetainList]

    foreach run $values {
        if {[catch {StagerConfiguration::appendRetainList $run} msg]} {
            if {[llength $oldList] > 0} {
                Configuration::SetRetainList $oldlist
            }
            error StagerConfiguration::InvalidParameter
        }
    }
}

# StagerConfiguration::getRetainList
#    Returns the current retension list.
#
proc StagerConfiguration::getRetainList {} {
    return [Configuration::get RetainedList]
}
# StagerConfiguration::clearRetainList
#    Clears the retension list.
#
proc StagerConfiguration::clearRetainList {} {
    Configuration::Set RetainedList ""
}

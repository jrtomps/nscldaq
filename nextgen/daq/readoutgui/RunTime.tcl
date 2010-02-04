#
#   This file is a package that can be used in a Tcl Server that
#   accepts sclclient connections to keep track fo the current Elapsed
#   time of a data taking run.
#   The package implements the global procs that are expected by the
#   sclclient application along with a few others that
#   - interpolate the run time between scaler updates.
#   - provide access to the run time.
#
#  The namespace RunTime is created to hold public functions
#  The elapsed time is held in floating point seconds with 0.5 second
#  resolution.
#

package provide RunTime 1.0
package require ReadoutGui
namespace eval ::RunTime {
    variable timerId -1;                      # interpolation timer.
}

#  On entry, set the elapsed run time to 0.

set ElapsedRunTime 0

#------------------------------------------------------------------------------
#
#   Entries required by sclclient:
#
proc BeginRun {} {
    set ::ElapsedRunTime 0
    ::RunTime::startClock
}
proc Update {} {
}
proc EndRun {} {
    ::RunTime::stopClock
}
proc ResumeRun {} {
    ::RunTime::startClock
}
proc PauseRun {} {
    ::RunTime::stopClock
}

# Since we start/stop the clock this should not happen.

proc RunInprogress {} {
}

#------------------------------------------------------------------------------
#
#  The entries below are responsible for maintaining the interpolated time.#
#  This is needed because the scaler buffer interval may be whatever the user
#  desired, but we want to maintain an update rate of 0.5 seconds so that
#  updates of the time every second will be reasonably right.
#

#
# Start the interpolation clock, if it's not already started.
# If the timer id is not -1, we schedule interpolate to
# run in 1/2 second from now:
proc ::RunTime::startClock {} {
    if {$::RunTime::timerId == -1} {
        set ::RunTime::timerId [after 500 ::RunTime::interpolate]
    }
}
#
#  IF the interpolation clock is running (timerId is not -1),
#  it is cancelled.
#
proc ::RunTime::stopClock {} {
    if {$::RunTime::timerId != -1} {
        after cancel $::RunTime::timerId
        set ::RunTime::timerId -1I
    }
    ReadoutGui::StopRunTimers
}
#  Periodic proc that adds 0.5 seconds to the ElapsedRunTime
#  and reschedules itself for 0.5 seconds later.
#
proc ::RunTime::interpolate {} {
    set ::RunTime::timerId [after 500 ::RunTime::interpolate]
    set ::ElapsedRunTime [expr {$::ElapsedRunTime + 0.5}]
}

#------------------------------------------------------------------------------
#
#   The package would not be complete without the ability to fetch the
#   clock, or format it in d hh:mm:ss.n
#


#
# Get the elapsed time in floating point seconds:
#
proc ::RunTime::elapsedTime {} {
    return $::ElapsedRunTime
}
#
#  Get the elapsed time formatted as
#   days hh:mm:ss.n
#
proc ::RunTime::formattedTime {} {
    set time $::ElapsedRunTime
    

    if {![string is double $time]} {
	set time 0
    }

    #  Seconds can include fractions:
    #
    
    set minutes [expr {int($time/60)}]
    set seconds [expr {$time - $minutes*60}]
    set hours   [expr {int($minutes/60)}]
    set minutes [expr {$minutes - $hours*60}]
    set days    [expr {int($hours/24)}]
    set hours   [expr {$hours - $days*24}]
                            
    return [format "%d %02d:%02d:%05.2f" $days $hours $minutes $seconds]
}

namespace eval ::RunTime {
    namespace export formattedTime elapsedTime
}


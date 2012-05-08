#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2009.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package provide s800callouts 1.0


puts "---------------------- S800 readout callouts script -------------"

#
#  Provides readout callout methods for the s800.
#
#  These just match the regular readoutcallouts methods but are in an
# s800 namespace.
# The assumption is that the s800 package is in our search path:
#
package require s800

namespace eval s800 {
    variable initialized false;	# If initialization has not taken place.
    variable s800        ""
    variable record
    variable unexpectedEnd [list]; # Command to execute on unexpected run end.
    variable runState      {unknown}
}

#
#  This should be explicitly called to initialize thec connection with
#  the remote run control system.
#
# @param host - host running the s800 readout.
# @param port - Port on which it listens for run control connections;
#
# @note - The s800 is put in to slave mode.
# @note - exit is renamed to s800::exit and 
#         1. exit is defined here to set the server to master mode
#            and close the connection prior to doing an s800::exitr
#         2. has a destroy handler set to do the same thing.
proc s800::Initialize {{host localhost} {port 8000}} {
    if {[catch {

    set s800::s800 [s800rctl %AUTO% -host $host -port $port]
    

    # Set the S800 in slave mode and, if it is running stop it to match
    # the current GUI state:



    set state [$::s800::s800 getState]
    if {$state eq "active"} {
	tk_messageBox -title  {s800 run active} \
	    -message {The S800 run is active and must be stopped first}
	::exit -1
    }
    $::s800::s800 setSlave; 
} msg]} {
	tk_messageBox -title {S800 protocol error} -message "Error communicating with S800@$host: $msg"
	::exit -1
    }

    # Remove the pause/resume button....once the GUI has time to start.

    after 1000 [list if {[winfo exists .pausres]} {grid forget .pauseres}]
    
    #  Set destroy handler:
    
    bind . <Destroy> [list s800::DestroyHandler %W]
    
    # Rename exit.
    
    rename ::exit ::s800::real_exit
    rename ::s800::exit ::exit
    
    toplevel .s800
    checkbutton .s800.record -text {Record Crate files} -variable ::s800::record \
	-onvalue 1 -offvalue 0
    pack .s800.record

    # Status data for the s800:

    label .s800statel -text {S800 state: }
    label .s800state  -textvariable ::s800::runState

    grid .s800statel -row 12 -column 5 -columnspan 2
    grid .s800state  -row 12 -column 7

    s800::monitorState 2000;	# Monitor th s800 state every nms.
    
    
}

##
# Self rescheduling proc that monitors the state of the S800.
# for now the entire body is in a catch loop and errors
# result in the state being set to 'unknown'.
#
# @param ms - Millieseconds of periodicity.
#
proc s800::monitorState ms {
    set status [catch {
	set s800State [$::s800::s800 getState]

	after $ms [list ::s800::monitorState $ms]

    } msg]

    # On failure, set state to unknown.

    if {$status} {
	set s800State Unknonwn
	tk_messageBox -title {S800 monitor failed} \
	    -message "monitor state failed: $msg no longer monitoring s800 state."
    }

    # Set the s800 state only if it changed.  Sinc there may
    # be traces on it we only want to fire them if necessary:

    if {$s800State ne $::s800::runState} {
	set ::s800::runState $s800State
    }

}

##
# Called in response to  a change in run state.  If the state change is
# out of 'active', and there's an unexpectedEnd handler it is called.
#
# @param name1 - Name of the state variable.
# @param name2 - array index or empty string if not an array.
# @param op    - Operation .. should be 'write'.
proc s800::stateChange {name1 name2 op} {
    set newState [set $name1]
    if {($newState ne "active") && ($::s800::unexpectedEnd ne "")} {

	uplevel #0 $::s800::unexpectedEnd $newState
	
    }
}

##
#   Called in respnose to destruction of the . widget...
#   or any widget in its tree.
#   Force the exit to go through our clean up handling.
#
# @param widget - the widget being destroyed.
#
proc s800::DestroyHandler {widget} {
    if {$widget eq "."} {
	exit
    }
}
#
#  Exit handler - puts the s800 back in master mode, if the socket
# is working then call the real exit
#
# @param status - the status of the real exit.
proc s800::exit {{status 0}} {
    if {$::s800::s800 ne ""} {
	if {[$::s800::s800 getState] eq "active"} { # end any active run
	    $::s800::s800 end
	}
	$::s800::s800 setMaster
	destroy $::s800::s800
	set ::s800::s800 ""

    }
    after 100 [list ::s800::real_exit $status]
}

##
# onBegin - start the s800 run.
#
proc s800::OnBegin {} {
    set title [ReadoutState::getTitle]
    set run   [ReadoutState::getRun]
    set home  $::env(HOME)
    if {[catch {
	$::s800::s800 setTitle $title
	$::s800::s800 setRun   $run
	$::s800::s800 setDestination $home
	if {$::s800::record} {
	    $::s800::s800 setRecordingOn
	} else {
	    $::s800::s800 setRecordingOff
	}
	$::s800::s800 begin

	# On successful begin establish a state handler for runState
	# 

	trace add variable ::s800::runState write ::s800::stateChange

    } msg]} {
	tk_messageBox -icon error -title "S800 readout GUI failed" \
	    -message "BEGIN Error communicating with the s800 readout - $msg - restart the s800 readout and this GUI"
	::s800::real_exit -1
    }
} 

##
# onEnd - stop the s800 run.
#
proc s800::OnEnd {} {
    if {[catch {
	$::s800::s800 end

	# cancel the variable trace.

	trace remove variable ::s800::runState write ::s800::stateChange

    } msg]} {
	tk_messageBox -icon error -title "S800 readout GUI failed" \
	    -message "END Error communicating with the s800 readout - $msg - restart the s800 readout and this GUI"
	::s800::real_exit -1
    }
    puts "Recording state [ReadoutState::getRecording]"
    if {[ReadoutState::getRecording] == 0} {
	puts "Attempting to increment run."
	ReadougGUIPanel::incrRun;	# change s800 run #

    }
}
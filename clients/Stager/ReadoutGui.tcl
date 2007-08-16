
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
puts "New Readout Gui file"

#  Package to provide a gui for the readout software.  Includes
#  a GUI to select the readout program. The assumptions are:
#
#  o Readout program will/may run on a remote system (over an rsh pipe).
#  o If readout program runs on a remote system, it completely shares
#    the user's directory structure.
#
#
package provide ReadoutGui 2.0
package require Configuration
package require Diagnostics
package require ExpFileSystem
package require ReadoutGUIPanel
package require ReadoutControl
package require Diagnostics
package require ReadoutState
package require DAQParameters
package require Experiment
package require bells

namespace eval ReadoutGui {
    variable ElapsedTimer     0;        #Number of seconds in run so far.
    variable RunStartedTime   0;        # When the run last (re)started in real time.
    variable TotalSegmentTime 0;        # Total elapsed time for run segments.

    variable guiBuiltCallback {};        # Callback when GUI is built.

}
# ReadoutGui::timestampOutput text
#    Called to write a timestamped string to the
#    gui text widget.  The string is of the form
#    "some text with a %s embedded" where the
#     %s will have a full timestamp substituted.
#
proc ReadoutGui::timestampOutput text {
	set stamp [clock format [clock seconds]]
	set output [format $text $stamp]
	ReadougGUIPanel::outputText "$output\n"
	
}
# ReadoutGui::setGuiBuiltCallback callback
#       Set a callback to be invoked when the Gui has been constructed.
# Parameters:
#   callback  - the new callback
#
proc ReadoutGui::setGuiBuiltCallback callback {
    set ::ReadoutGui::guiBuiltCallback $callback
}
# ReadoutGui::NeedHostandProgram
#    Ensure the user enters a host and program.
#
proc ReadoutGui::NeedHostandProgram {} {
    tk_dialog .needinfo {Need Host and program} \
	{In order to start the readout program, you must provide a host and program via the File->New... menu} \
	error 0 Dismiss
}
# ReadoutGui::OvewriteCheck
#
#  Display a dialog that asks what to do if we would
#  otherwise overwrite event data?  Note that if there
#  is no event data to overwrite, the dialog is not displayed.
#
# Returns:
#     0    - Don't start the run.
#     1    - Start the run but don't record data.
#     2    - Start the run and overwrite the data.
#
proc ReadoutGui::OverwriteCheck {} {
    set run [ReadougGUIPanel::getRunNumber]
    if {[Experiment::RunFileExists $run]} {
	set testrun $run
	incr testrun
	while {[Experiment::RunFileExists $testrun]} {
	    incr testrun
	}
	set message \
	    "WARNING: Event data for run $run already exists! "
	append message \
	    " If you continue to start this run with event "
	append message "recording enabled, you will overwrite "
	append message "this file. "
	append message "Note that the next run number without "
	append message "recorded event data is $testrun. "
	append message  "Please select an "
	append message "appropriate action:"
	return [tk_dialog .ovewrite "!!Overwrite!!" $message \
		    warning 0 \
		    "Don't start" \
		    "Start without recording" \
		    "Start and overwrite"]

    } else {
	return 2;			# No problems starting run.
    }

}
#  Sometimes we need to substitute a callback with a
# dononthing.  This proc is the donothing.
#
proc ReadoutGui::NoOp {args} {}

# ReadoutGui::UpdateLinks runfiles
#
#    Update links: called to update the
#    event file links in the current directory.
#    the parameter is the set of event file segments.
# Parameters:
#     runfiles - full paths to the run files.
#
proc ReadoutGui::UpdateLinks {segments} {
    set ExpCurrent [ExpFileSystem::WhereisCurrentData]
    foreach segment $segments {
	set basename [file tail $segment]
	set linktarget [file join $ExpCurrent $basename]
	if {![file exists $linktarget] } {
	    catch {exec ln -s $segment $linktarget}
	}
    }
}
# ReadoutGui::RdoCommand cmd
#     Called when a command is sent to the readout program.
#     The command is echoed on the output
# Parameters:
#  cmd   - The command that was sent.
#
proc ReadoutGui::RdoCommand {cmd} {
    ReadougGUIPanel::outputText "$cmd\n"
}
# ReadoutGui::SaveSettings
# Save Readout Settings to configuration database and write
# the entire configuration to the experiment root's .readoutconfig file.
#
#
proc ReadoutGui::SaveSettings {} {
    set file [file join [ExpFileSystem::GetStage] .readoutconfig]
    ::ReadoutState::setTitle [ReadoutControl::GetTitle]
    ::ReadoutState::setRun   [ReadoutControl::GetRun]
    ::ReadoutState::setScalerCount  [ReadoutState::getScalerCount]
    ::ReadoutState::setScalerPeriod [ReadoutState::getScalerPeriod]
    set host   [ReadougGUIPanel::getHost]
    set rdpath [ReadougGUIPanel::getPath]
    if {[catch {::DAQParameters::sourceHostIs $host} msg]} {
	tk_dialog .badhost {Bad Host} \
	    "Readout program run host: $host is bad: $msg" \
	    error 0 Continue
	return 0
    }
    if {[catch {::DAQParameters::readoutPathIs $rdpath} msg]} {
	tk_dialog .badpath {Bad ReadoutPath} \
	    "Readout program path $rdpath is invalid: $msg" \
	    error 0 Continue
	return 0
    }

    set fd [open $file w]
    ::Configuration::writeConfigFile $fd
    close $fd
    return 1
}
# ReadoutGui::SourceFile filename
#   Proc to source a file into the readout program.
#   The file contains arbitrary readout program commands.
#   this is especially intended for production readout.
# Parameters:
#  filename   - Name of the file to source into the readout program.
#
proc ReadoutGui::SourceFile {filename} {
    if {$filename != ""} {
	set sfd [open $filename r]
	set contents [read $sfd]
	close $sfd
	ReadoutControl::SendCommand $contents
    }
}

# ReadoutGui::TimeToEndRun seconds
#    Returns true if it is time to end a timed run.
# Parameters
#  seconds   - The number of seconds the run has been active
#              This should not count paused time.
#
#
proc ReadoutGui::TimeToEndRun {seconds} {
    set runLength [ReadougGUIPanel::getRequestedRunTime]
    if {$seconds >= $runLength} {
	return 1
    } else {
	return 0
    }
}
# ReadoutGUI::ClearElapsedTime
#   Clear the elapsed time label in the readout gui panel.
#
proc ReadoutGui::ClearElapsedTime {} {
    ::ReadougGUIPanel::setActiveTime 0 0 0 0
    set ::ReadoutGui::TotalSegmentTime 0
}
# ReadoutGui::StartElapsedTimer
#      Starts the elapsed timer. This can be called
#      either after a temporary StopElapsedTimer
#      or after a ClearElapsedTimer.
#
proc ReadoutGui::StartElapsedTimer {} {
    set ::ReadoutGui::RunStartedTime [clock seconds]
    set ::ReadoutGui::ElapsedTimer [after 1000 ReadoutGui::SecondElapsed]
}
# ReadoutGui::StopElapsedTimer
#      - The elapsed timer is cancelled.
#      - The run time since RunStartedTime is added to
#        TotalSegmentTime
#      - RunStartedTime is cleared.
# All of this allows a resumption of the timer without counting the
# time the timer was disabled.
#
proc ReadoutGui::StopElapsedTimer {} {
    if {$::ReadoutGui::ElapsedTimer !=0} {
	after cancel $::ReadoutGui::ElapsedTimer
	set ::ReadoutGui::ElapsedTimer 0
    }
    incr ::ReadoutGui::TotalSegmentTime [expr {[clock seconds] - $::ReadoutGui::RunStartedTime}]
    set  ::ReadoutGui::RunStartedTime 0

}
# ReadoutGui::SecondElapsed
#     Called when a second has elapsed.
#     The total run time is computed and displayed in the GUI Panel.
#     The status window is updated to reflect the amount of event data
#     acquired this run.
#
#
proc ReadoutGui::SecondElapsed {} {

    # Figure out how long we've run.

    set now [clock seconds]
    set elapsed [expr {$now - $::ReadoutGui::RunStartedTime
			    + $::ReadoutGui::TotalSegmentTime}]
    set elapsedSeconds $elapsed
    set Sec     [expr {$elapsed % 60}]
    set elapsed [expr {$elapsed/60}]
    set Min     [expr {$elapsed % 60}]
    set elapsed [expr {$elapsed/60}]
    set Hrs     [expr {$elapsed % 24}]
    set Days    [expr {$elapsed / 24}]
    ::ReadougGUIPanel::setActiveTime $Days $Hrs $Min $Sec

    #
    #  Update the status line for the event file:
    #  We need to sum all the sizes for all the segments.
    #  summing is done in Kbyte units in order to prevent overflows.
    #  We also invoke UpdateLinks to ensure that all event file links
    #  are valid.
    #
    set eventDir [ExpFileSystem::WhereisCurrentEventData]
    set run [::ReadoutState::getRun]
    set filepat [file join $eventDir [::ExpFileSystem::GenRunFileBase $run]]
    append filepat * .evt
    set    segments [glob -nocomplain $filepat]
    set    size     0.0
    set    nsegments [llength $segments]
    foreach segment $segments {
	if {![catch {file size $segment} segsize]} {
	    set size [expr {$size + $segsize/1024.0}]
	}
    }
    set size [expr {$size/1024.0}]
    if {$nsegments > 0} {
	ReadoutGui::UpdateLinks $segments
	set status [format "Run %d recorded in %d segments totalling %9.2f Mbytes" \
			 $run $nsegments $size]
    } else {
	set status "No run file segments for run $run yet"
    }
    ReadougGUIPanel::setStatusLine $status


    #
    # Repropagate self:
    #
    set ReadoutGui::ElapsedTimer [after 1000 ReadoutGui::SecondElapsed]

    #
    #  If the run is timed, see if it's time to end it:
    #  Note that End stops the elapsed time timer.  That's why we
    #  repropogated first.
    #

    if {[ReadougGUIPanel::isTimed]} {
	if {[ReadoutGui::TimeToEndRun $elapsedSeconds]} {
	    ReadoutGui::End
	}
    }

}
# ReadoutGui::ReadoutInput fd
#     Callback for when input comes from the readout program.
#     reads the data and writes it to the panel's text.
# Parameters:
#  fd   - File descriptor open on the readout program's pipe.
#
proc ReadoutGui::ReadoutInput {fd} {
    variable OutputWidget
    if {![eof $fd]} {
	set text [read $fd 1000]
	ReadougGUIPanel::outputText $text ;# Assume fd is nonblock.
	update idle
    }
}

# ReadouGuit::HaveReadout
#
#  local Utility to determine if the readout controller already has the host
#  and program.
#
proc ReadoutGui::HaveReadout {} {
    set host [ReadougGUIPanel::getHost]
    set path [ReadougGUIPanel::getPath]

    if {($host == "") || ($path == "")} {
	return 0
    } else {
	return 1
    }
}

# ReadougGui::onExit
#  Exit callback handler established to be called whenthe program exits.
#
#
proc ReadoutGui::onExit {} {

    ReadoutGui::SaveSettings
    ReadoutControl::SetOnExit ::ReadoutGui::NoOp
    ReadoutControl::ExitReadoutProgram
    exit
}
# ReadoutGui::CleanupRun State
#  This is called when the readout program exits.  It determines
#  what if any cleanup is needed as a result of readout dying potentially
#  in the middle of a run.
#Parameters:
#  State - State Readout was in when abnormal end occured
proc ReadoutGui::CleanupRun {State} {
    # If is not ended, need to do emergency end run.
    #

    if {$State == "Active"} {
	Experiment::EmergencyEnd
	if {[ReadoutControl::isTapeOn]} {
	    set nrun [ReadoutControl::GetRun]
	    incr nrun
	    ReadoutControl::SetRun $nrun
	    ReadougGUIPanel::setRun $nrun
	}
    }
}
# ReadoutGui::ReadoutExited
#
#   The readout program exited.  We need to ghost all the stuff
#  which enabled on active.
#  We need to change the restart menu entry to a Start entry.
#Parameters:
#   RunStateOnExit  - the state of the readout program on exit.
proc ReadoutGui::ReadoutExited {RunStateonExit} {
    timestampOutput "%s : The Readout program exited!"
    ReadoutGui::CleanupRun $RunStateonExit
    ReadougGUIPanel::readoutNotRunning
    bells notify -interval 2500 -pattern {100 200 400 500}
    set doWhat [tk_dialog .rdoexited {Readout Exited} \
		{The readout program exited.  What would you like to do?} \
		 warning 0 Exit {Attempt to Restart} {Do Nothing}]
    notify destroy
    switch -exact $doWhat {
	0 {
	    ReadoutGui::onExit
	}
	1 {
	    ReadoutGui::doRestart
	}
	2 {
	}
    }

}
# ReadoutGui::EmergencyExit
#    Callback hooked to widget deletion.
#
proc ReadoutGui::EmergencyExit {} {
    ReadoutGui::CleanupRun [ReadoutControl::getReadoutState]
    ReadoutControl::SetOnExit ::ReadoutGui::NoOp
    ReadoutControl::ExitReadoutProgram
    exit
}
# ReadoutGui::Start
#  Starts/restarts the front end program.
#
proc ReadoutGui::Start {} {
    if {![HaveReadout]} {
	::ReadoutGui::NeedHostandProgram
	return
    }

    if {![ReadoutGui::SaveSettings]} {
	return
    }
    ReadoutControl::SetReadoutProgram [DAQParameters::getSourceHost] \
				      [DAQParameters::getReadoutPath]
	timestampOutput "%s : Starting the Readout program."
    ReadoutControl::StartReadoutProgram
    ReadougGUIPanel::readoutRunning
    ReadougGUIPanel::runIsHalted
    ReadoutControl::ShowAll
    ReadoutControl::SetOnExit ::ReadoutGui::ReadoutExited
}
# ReadoutGui::doRestart
#     Do the actual restart of a readout program without
#     prompting etc.
#
proc ReadoutGui::doRestart {} {
    if {![ReadoutGui::SaveSettings]} {
	return
    }
    ReadoutControl::SetOnExit ::ReadoutGui::NoOp
    ReadoutControl::ExitReadoutProgram
    ReadoutGui::StopRunTimers
    ReadoutGui::Start

}
# ReadoutGui::Restart
#     Restarts the front end program.  This involves:
#     - Confirming with the user that this is what they want done.
#     - Stopping the current readout program.
#     - Starting the new readout program.
#
proc ReadoutGui::Restart {} {
    if {![HaveReadout]} {
	ReadoutGui::NeedHostandProgram
	return
    }
    set restart \
	    [tk_dialog .restart "Restart" \
    "If you restart any currently active run will be ended.  Do you want to Restart?" \
	       warning 1 Restart Cancel]
    if {$restart == 0} {
		timestampOutput "%s : Restarting the readout program."
		ReadoutGui::doRestart
    }


}


# ReadoutGui::StartRunTimers
#     Sets up timers and other periodic
#     stuff required to know the elapsed run
#     time as well as to end a timed run.
#
proc ReadoutGui::StartRunTimers {} {


    ReadoutGui::StartElapsedTimer
    ReadougGUIPanel::outputText "Run Starting\n"
    ReadoutGui::SaveSettings
    if {[ReadoutControl::isTapeOn]} {
	ReadougGUIPanel::isRecording
	# $EventStatusLineWidget config -bg green
	# $OutputWidget config -bg green
    } else {
	ReadougGUIPanel::notRecording
	# set EventFileStatusLine ""
    }
}
# ReadoutGui::Begin
#      Called when the GUI wants to start a run.
#
#
proc ReadoutGui::Begin {} {

    #  If necessary, check to see what to do
    #  if the user is ovewriting:
    #

    if {[ReadougGUIPanel::recordData]} {
	ReadoutState::enableRecording
	ReadoutControl::EnableTape
	set action [ReadoutGui::OverwriteCheck]
	if {$action == 0} {;	# Run aborted.
	    return
	}
	if {$action == 1} {;	# Turn off tape.. but run.
	    ReadoutControl::DisableTape
	    ReadougGUIPanel::recordOff
	}
	#  2 overwrite if needed.


    } else {
		ReadoutState::disableRecording
		ReadoutControl::DisableTape
    }
    # Update the scaler info with the GUI's idea of what it should be.

    set scalerinfo [ReadougGUIPanel::getScalers]
    ReadoutState::setScalerCount  [lindex $scalerinfo 0]
    ReadoutState::setScalerPeriod [lindex $scalerinfo 1]

    ReadoutControl::SetRun   [::ReadougGUIPanel::getRunNumber]
    ReadougGUIPanel::runIsActive
	timestampOutput "%s : Starting a new run"
    ReadoutControl::SetTitle [::ReadougGUIPanel::getTitle]
    ReadoutControl::Begin
    ReadoutControl::ShowAll

    # If a timed run, start the end run timer.
    # Also  start the timers needed to show the status line and
    # increment the elapsed run time.

    if {[ReadougGUIPanel::isTimed]} {
	puts "Timed"
	ReadoutState::TimedRun
	ReadoutState::setTimedLength [ReadougGUIPanel::getRequestedRunTime]
    } else {
	puts Untimed
	ReadoutState::notTimedRun
    }
    ReadoutGui::ClearElapsedTime;    # NO paused segments, new run.
    ::ReadoutGui::StartRunTimers

}
# ReadoutGui::Pause
#    Called to pause a run.  This can only happen if the
#    pause button is clicked as there is not a concept now
#    of a timed pause.
#
proc ReadoutGui::Pause {} {
    ReadoutGui::StopRunTimers
    ReadougGUIPanel::runIsPaused
    ReadougGUIPanel::setStatusLine {Run paused}
	timestampOutput "%s : Pausing the run"
    ReadoutControl::Pause
}
# ReadoutGui::Resume
#   Called to resume a run via the GUI.
#
proc ReadoutGui::Resume {} {
	timestampOutput "%s : Resuming the run"
    ReadoutControl::Resume
    ReadoutControl::ShowAll
    ReadougGUIPanel::runIsActive
    ReadougGUIPanel::setStatusLine {Run started}
    ReadoutGui::StartElapsedTimer;    # Accumulate active time across pause.
}
#  ReadoutGui::StopRunTimers
#  Stops the timers associated with a run.
#
#
proc ReadoutGui::StopRunTimers {} {
    ReadoutGui::StopElapsedTimer
}
# ReadoutGui::End
#    Called to end a run.  This is called whether the run
#    was ended due to the pushbutton or due to timed run
#    being done.
#
proc ReadoutGui::End {} {
    ReadoutGui::StopRunTimers
    ReadougGUIPanel::runIsHalted
	timestampOutput "%s : Ending the run"
    ReadoutControl::End
    if {[ReadoutControl::isTapeOn]} {
	ReadougGUIPanel::incrRun
	ReadoutControl::SetRun [ReadougGUIPanel::getRunNumber]
    }
    ReadougGUIPanel::normalColors

    ReadougGUIPanel::setStatusLine {Run Ended}
    ReadoutControl::ShowAll
    ReadoutGui::SaveSettings

}
#
#  Top level GUI for the run control.
#
proc ReadoutGui::ReadoutController {topname} {



    ReadoutControl::GetRun     ;# Force package load.

    #   Build the window and menu structure.
    #

    # there are two possibilities.  The topname is . in which case
    # the interface is built into the base tk window, or it isn't in
    # which case we'll pop up our own toplevel and build the gui into that.
    # regardless, a destroy handler is established to ensure that
    # any readout program is killed off.
    #
    if {$topname ne ""} {
	toplevel $topname
	bind $topname <Destroy> ::ReadoutGui::EmergencyExit
	set topprefix $topname
    } else {
	bind . <Destroy> ::ReadoutGui::EmergencyExit
	set topprefix ""
	set topname .
    }
    wm title $topname "Run Control"
    ::ReadougGUIPanel::init $topname
    if {$::ReadoutGui::guiBuiltCallback != ""} {
	$::ReadoutGui::guiBuiltCallback $topname
    }


    # Recover configuration settings for scaler, title, run


    set title   [ReadoutState::getTitle]
    set run     [Experiment::BiggestRun]
    set scalers [ReadoutState::getScalerCount]
    set period  [ReadoutState::getScalerPeriod]
    set host    [DAQParameters::getSourceHost]
    set path    [DAQParameters::getReadoutPath]
    set recording [ReadoutState::getRecording]
    set timedrun  [ReadoutState::isTimedRun]
    set timedlen  [ReadoutState::timedLength]



    #  Set up the GUI  with the values of the configuration
    # settings:

    ReadougGUIPanel::setTitle   $title
    ReadougGUIPanel::setRun     $run
    ReadougGUIPanel::setHost    $host
    ReadougGUIPanel::setPath    $path
    ReadougGUIPanel::setScalers $scalers $period
    if {$recording} {
	ReadougGUIPanel::recordOn
    } else {
	ReadougGUIPanel::recordOff
    }
    ReadougGUIPanel::setTimed $timedrun
    if {$timedlen ne ""} {
	ReadougGUIPanel::setRequestedRunTime $timedlen
    }

    # Setup the readout control package with this as well.

    ReadoutControl::SetTitle $title
    ReadoutControl::SetRun   $run




    # Establish callback handlers for input from the Readout Program:
    #
    ReadoutControl::SetOnInput ReadoutGui::ReadoutInput
    #
    #  Let the experiment manager tell us what to use as the first run.
    #

    ReadoutControl::SetOnCommand ::ReadoutGui::RdoCommand

        #  Set the callbacks for the control buttons

    ReadougGUIPanel::startStopCallbacks   ::ReadoutGui::Begin ::ReadoutGui::End
    ReadougGUIPanel::pauseResumeCallbacks ::ReadoutGui::Pause ::ReadoutGui::Resume
    ReadougGUIPanel::setExitCallback      ::ReadoutGui::onExit
    ReadougGUIPanel::setStartCallback     ::ReadoutGui::Start
    ReadougGUIPanel::setRestartCallback   ::ReadoutGui::Restart
    ReadougGUIPanel::setSourceCallback    ::ReadoutGui::SourceFile

    # set other callbacks and initialize other packages.

    Experiment::Register
    ReadoutControl::SetOnInput ::ReadoutGui::ReadoutInput
    ReadoutControl::SetOnExit  ::ReadoutGui::NoOp

    # If a readout program has been unambiguously defined, start it.

    if {[::ReadoutGui::HaveReadout]} {
	::ReadoutGui::Start
    } else {
	::ReadougGUIPanel::ghostBegin
	::ReadougGUIPanel::readoutNotRunning
    }


}

    # Exported functions:

namespace eval ReadoutGui {
    namespace export  GetReadoutSpec
    namespace export  ReadoutController
    namespace export  ReadoutInput
    namespace export  DisableEventRecording
    namespace export  EnableEventRecording
}



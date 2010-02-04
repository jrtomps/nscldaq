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


package provide ReadoutControl 2.0
package require rsh
package require ExpFileSystem
package require ReadoutState
package require DAQParameters
package require ScalerParameterGUI

namespace eval ReadoutControl {
    variable State   NotLoaded
    variable PipeFd   " "
    variable Taping   0
    
    variable sclclientPid 0

    #
    #   Callback procs for state change.
    #
    variable BeginCallback   ReadoutControl::NoOp
    variable EndCallback     ReadoutControl::NoOp
    variable PauseCallback   ReadoutControl::NoOp
    variable ResumeCallback  ReadoutControl::NoOp
    variable LoadCallback    ReadoutControl::NoOp
    variable ExitCallback    ReadoutControl::CmdNoOp
    variable InputCallback   ReadoutControl::Flush
    variable CommandCallback ReadoutControl::CmdNoOp
}
# ReadoutControl::getReadoutState
#    Get the current readout controller state (ReadoutControl::State)
#
proc ReadoutControl::getReadoutState {} {
    return $::ReadoutControl::State
}
# ReadoutControl::PathFind file
#     Locates a file along the auto_path.
# Parameters:
#  file   - File to locate (this is the file tail).
# Returns:
#   Full path to the file or "" if the file does not exist.
#
proc ReadoutControl::PathFind {file} {
    foreach path  $::auto_path {
        set name [file join $path $file]
        if {[file exists $name] } {
            set here [pwd]
            cd $path
            set abspath [file join [pwd] $file]
            cd $here
            return $abspath
        }
    }
    return ""
}
# ReadoutControl::isExecuting
#    Returns true if the readout program is running.
#
proc ReadoutControl::isExecuting {} {
    if {$::ReadoutControl::State == "NotLoaded"} {
        return 0
    } else {
        return 1
    }
}
#
#  The NoOp proc is the default callback.
#  It can be replaced with application specific callbacks as well.
#
proc ReadoutControl::NoOp {} { }
proc ReadoutControl::CmdNoOp {command} {}
#
# The Flush proc is the default callback for input on the
# readout pipe. Data is read and tossed. (Assumption is that
# fd is in nonblocking mode.
#

proc ReadoutControl::Flush {fd} {
    read $fd 100
}
#
# ReadoutControl::SendCommand string:
#   Sends a command string to the Readout Program:
# Parameters:
#    string:  The string to send.

proc ReadoutControl::SendCommand {string} {
    puts $::ReadoutControl::PipeFd $string
    flush $::ReadoutControl::PipeFd

    if {[info proc $::ReadoutControl::CommandCallback] != ""} {
        $::ReadoutControl::CommandCallback $string
    }
}
#
#  ReadoutControl::ReadoutFileEvent
#        Called when a data is available on the
#        Readout input pipe... calls InputCallback.
#
proc ReadoutControl::ReadoutFileEvent {} {

    if {[eof $::ReadoutControl::PipeFd] } {
        catch "close $::ReadoutControl::PipeFd"
        set priorstate $ReadoutControl::State
        set ::ReadoutControl::State NotLoaded
        $::ReadoutControl::ExitCallback $priorstate
        update idle
        return
    }
    $::ReadoutControl::InputCallback $::ReadoutControl::PipeFd
}
    #
    # Utility to set the 'state' variables down into the Readout's
    # pipe.  The assumption is that the run is halted.
    #
proc ReadoutControl::SetState {} {
    set RunTitle     [ReadoutState::getTitle]
    set RunNumber    [ReadoutState::getRun]
    set Scalers      [ReadoutState::getScalerCount]
    set ScalerPeriod [ReadoutState::getScalerPeriod]

    set ok [catch {
	::ReadoutControl::SendCommand "set run $RunNumber"
        ::ReadoutControl::SendCommand "set title \"$RunTitle\""

	::ReadoutControl::SendCommand "set scaler $Scalers"
        ::ReadoutControl::SendCommand "set frequency $ScalerPeriod"
        flush $::ReadoutControl::PipeFd
    } emsg]
    if {$ok} {
        puts $emsg
    }
}
#
#
#  The following procs allow the callbacks to be replaced:
#    SetOnBegin - Called just prior to starting a run.
#    SetOnEnd   - Called just after a run is ended.
#    SetOnPause - Called just after a run is paused.
#    SetOnResume- Called Just before a run is resumed.
#    SetOnLoad  - Called just after the Readout program has been started.
#    SetOnExit  - Called just after the Readout program has exited.
#
proc ReadoutControl::SetOnBegin {onbegin} {
    if {[info proc $onbegin] != ""} {
        set ::ReadoutControl::BeginCallback $onbegin
    }
}
proc ReadoutControl::SetOnEnd {onend} {
    if {[info proc $onend] != ""} {
        set ::ReadoutControl::EndCallback $onend
    }
}
proc ReadoutControl::SetOnPause {onpause} {

    if {[info proc $onpause] != ""} {
        set ::ReadoutControl::PauseCallback $onpause
    }
}
proc ReadoutControl::SetOnResume {onresume} {
    if {[info proc $onresume] != ""} {
        set ::ReadoutControl::ResumeCallback $onresume
    }
}
proc ReadoutControl::SetOnLoad {onload} {
    if {[info proc $onload] != ""} {
        set ::ReadoutControl::LoadCallback $onload
    }
}
proc ReadoutControl::SetOnExit {onexit} {
    if {[info proc $onexit] != ""} {
        set ::ReadoutControl::ExitCallback $onexit
    }
}
proc ReadoutControl::SetOnInput {oninput} {
    if {[info proc $oninput] != ""} {
        set ::ReadoutControl::InputCallback $oninput
    }
}
proc ReadoutControl::SetOnCommand {onCommand} {

    if {[info proc $onCommand] != ""} {
        set ::ReadoutControl::CommandCallback $onCommand
    }
}


    #
    #   The following procedures define the readout program and
    #   start it up and stop it.
    #
proc ReadoutControl::SetReadoutProgram {host path} {

    ::DAQParameters::sourceHostIs $host
    ::DAQParameters::readoutPathIs $path

}
# ReadoutControl::ExitReadoutProgram
#     Requests the readout program exit.
#
proc ReadoutControl::ExitReadoutProgram {} {

    if {$::ReadoutControl::sclclientPid != 0} {
	catch {exec kill -9 $::ReadoutControl::sclclientPid}
	set sclclientPid 0
    }

    if {$::ReadoutControl::State == "NotLoaded"} return
    if {$::ReadoutControl::State != "NotRunning"}  {
	flush stdout
	::ReadoutControl::End
    }
    catch {
        ::ReadoutControl::SendCommand  exit
    }
    set ::ReadoutControl::State NotLoaded
    $::ReadoutControl::ExitCallback $::ReadoutControl::State
    update idle
    
    catch {
	fileevent $::ReadoutControl::PipeFd {}
	close $::ReadoutControl::PipeFd
    }
}
# ::ReadoutControl::StartReadoutProgram
#      Starts the readout program.
#
proc ::ReadoutControl::StartReadoutProgram {} {

    if {$::ReadoutControl::State != "NotLoaded"} {
        ::ReadoutControl::ExitReadoutProgram
    }

    #  Attempt to locate the environment setup script.
    #
    set script [::ReadoutControl::PathFind RunReadout.sh]

    # In order to ensure that Readout cannot be modified by the
    # user while we run it, copy the user's readout program to
    # ~/stagearea/.readout

    set cwd [pwd]
    cd ~
    set home [pwd]
    cd $cwd

    set DestDir $home/.readout
    file mkdir $DestDir

    set ReadoutProgram [DAQParameters::getReadoutPath]
    set ReadoutHost    [DAQParameters::getSourceHost]

    file copy -force $ReadoutProgram $DestDir
    set Executable [file join $DestDir  [file tail $ReadoutProgram]]
    file attributes $Executable -permissions u+x,g+x,o+x
    #
    #   Break the readout file apart into it's path and name.

    set path [file dirname $ReadoutProgram]
    set name [file tail    $Executable]

	#  Now do the rsh.

    if {$script == ""} {	;# Run directly.
	set fd [rsh::rshpipe $ReadoutHost $Executable r+]

    } else {		;# Run via env setup script.
	set fd [rsh::rshpipe $ReadoutHost "$script $path $Executable" r+]	

    }
    if {![eof $fd]} {
	set ::ReadoutControl::PipeFd $fd
        fconfigure $::ReadoutControl::PipeFd -blocking 0 -buffering none
	fileevent $::ReadoutControl::PipeFd readable ReadoutControl::ReadoutFileEvent
        $::ReadoutControl::LoadCallback
	set ::ReadoutControl::State NotRunning

	# Stop the current scclient and start a new one taking data from
	# ReadoutHost and connected to us at TclServerPort
	# This is probably the simplest as I don't think the
	# readout program will get restarted all that much.. and this
	# allows the software to deal with the host changing or not being known
	# when we start.
	# Assumptions:
	#   ::TclServerPort  - The port we're listening on.
	#   ::bindir         - the directory in which sclclient lives.
	#   The correct url is tcp://$ReadoutHost:2602/
	#

	if {$::ReadoutControl::sclclientPid != 0} {
	    catch {exec kill -9 $::ReadoutControl::sclclientPid};  # Kill and errors don't matter.
	}
	set whoami $::tcl_platform(user)
	set ::ReadoutControl::sclclientPid \
	    [exec $::bindir/sclclient -p $::TclServerPort -s tcp://$ReadoutHost/$whoami &]
	
    } else {
        close $fd
    }
}
    #
    #   The following allow the Run parameters to be set.  Legal only
    #   If the run is halted. (State == NotRunning).
    #

# ReadoutControl::SetRun number
#    sets a new run number.
# Parameters:
#   number - the new run number.

proc ReadoutControl::SetRun {number} {

    if {($::ReadoutControl::State == "NotRunning") ||
        ![::ReadoutControl::isExecuting]} {
        ::ReadoutState::setRun $number
    } else {
	error "Run is not halted"
    }
}
# ReadoutControl::GetRun
#     Return the current run number.
#
proc ReadoutControl::GetRun {} {

    return [::ReadoutState::getRun]
}
# ReadoutControl::SetTitle
#      set a new run title.
# Parameters:
#   Title   - The new title string.
#
proc ReadoutControl::SetTitle {Title} {

    if {$::ReadoutControl::State == "NotRunning" ||
        ![::ReadoutControl::isExecuting]} {
        ::ReadoutState::setTitle $Title
    } else {
	error "Run is not halted"
    }
}
# ReadoutControl::GetTitle
#        Gets the current value of the title.
#
proc ReadoutControl::GetTitle {} {

    return [::ReadoutState::getTitle]
}
#  ReadoutControl::EnableTape
#       Enable event recording.
#
proc ReadoutControl::EnableTape {} {
    if {$::ReadoutControl::State == "NotRunning" ||
        ![::ReadoutControl::isExecuting]} {
	set ::ReadoutControl::Taping 1
    } else {
	error "Run is not halted"
    }
}
# ReadoutControl::DisableTape
#     Disable event recording.
#
proc ReadoutControl::DisableTape {} {
    if {$::ReadoutControl::State == "NotRunning"  ||
        ![::ReadoutControl::isExecuting]} {
	set ::ReadoutControl::Taping 0
    } else {
	error "Run is not halted."
    }
}
# ReadoutControl::isTapeOn
#     Returns true if event recording is enabled.
#
proc ReadoutControl::isTapeOn {} {

    return $::ReadoutControl::Taping
}
#
#   The following procedures manage run state transitions.
#   Only legal transitions are allowed.
#

# ReadoutControl::Begin
#    Tells the readout software to start a run.
#
proc ReadoutControl::Begin {} {

    if {$::ReadoutControl::State != "NotRunning"} {
	error "Run is active"
    } else {
	::ReadoutControl::SetState
	$::ReadoutControl::BeginCallback

        catch {
	    ::ReadoutControl::SendCommand "begin"
    	flush $::ReadoutControl::PipeFd
	}
	set ::ReadoutControl::State "Active"
    }
}
# ReadoutControl::End
#     Tells the readout software to end a run.
#
proc ReadoutControl::End {} {
    if {$::ReadoutControl::State == "NotRunning" } {
	error "Run is already halted"
    } else {
	catch {
	    ::ReadoutControl::SendCommand "end"
            flush $::ReadoutControl::PipeFd
        }
	set ::ReadoutControl::State "NotRunning"
        $::ReadoutControl::EndCallback
    }
}
# ReadoutControl::Pause
#     Tells the readout software to pause a run.
#
proc ReadoutControl::Pause {} {
    if {$::ReadoutControl::State != "Active"} {
        error "Run is not active"
    } else {
	catch {
            ::ReadoutControl::SendCommand "pause"
	    flush $::ReadoutControl::PipeFd
	}
	set ::ReadoutControl::State "Paused"
        $::ReadoutControl::PauseCallback
    }
}
# ReadoutControl::Resume
#    Tells the readout software to resume a run.
#
proc ReadoutControl::Resume {} {

    if {$::ReadoutControl::State != "Paused" } {
	error "Run is not paused"
    } else {
	$::ReadoutControl::ResumeCallback
        catch {
	    ::ReadoutControl::SendCommand "resume"
	    flush $::ReadoutControl::PipeFd
	}
        set ::ReadoutControl::State "Active"
    }
}

# ReadoutControl::ShowAll
#     Sends a show all command to the readout software.#
#     This should result in a bunch of input that can be
#     captured by the callback software and ..well usually just
#     displayed.
#     This is currently disabled in order to be compatible with the
#     production readout software.. that does not have show all.
#
proc ReadoutControl::ShowAll {} {

    # For now a no-op.
}

namespace eval ReadoutControl {
    namespace export SetOnCommand
    namespace export SetOnBegin SetOnEnd SetOnPause SetOnResume
    namespace export SetOnLoad SetOnExit SetOnInput
    namespace export SetReadoutProgram ExitReadoutProgram StartReadoutProgram
    namespace export SetRun GetRun SetTitle GetTitle EnableTape DisableTape
    namespace export isTapeOn
    namespace export Begin End Pause Resume
    namespace export ReadoutHost ReadoutProgram
    namespace export RunTitle RunNumber Taping Period
}

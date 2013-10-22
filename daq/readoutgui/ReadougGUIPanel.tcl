# ReadoutGUIPanel.tcl --
#
# UI generated by GUI Builder Build 107673 on 2005-08-09 08:48:08 from:
#    //homedir/home2/fox/My Documents/DAQDocs/2005a/daq/DirectoryStructure/code/ReadoutGUIPanel.ui
# This file is auto-generated.  Only the code within
#    '# BEGIN USER CODE'
#    '# END USER CODE'
# and code inside the callback subroutines will be round-tripped.
# The proc names 'ui' and 'init' are reserved.
#

package require Tk 8.4
package require ScalerParameterGUI
package require selectReadoutDialog
# Declare the namespace for this dialog
namespace eval ReadoutGUIPanel {
    variable OutputSavedLines 500
    variable TimedRun          0
    variable RecordData        0
    variable ScalerChannels    0
    variable ScalerPeriod      2
    variable UseTestColors     0
    variable TestNonRecordingBg red
    variable TestRecordingBg    blue
    variable TestNonRecordingFg blue
    variable TestRecordingFg    red

    variable sourceDefaultPath ./

    #  Callbacks:

    variable beginCallback    {}
    variable endCallback      {}
    variable pauseCallback    {}
    variable resumeCallback   {}
    variable restartCallback  {}
    variable startCallback    {}
    variable exitCallback     {}
    variable sourceCallback   {}

    variable buttonRelief     raised
    variable buttonBorder     1
}

# Source the ui file, which must exist
source [file join [file dirname [info script]] ReadoutGUIPanel_ui.tcl]

# BEGIN USER CODE
package provide ReadoutGUIPanel 1.0

#  This code ensures that pkg_mkIndex does not choke
# on this.

if {[info var ::argv0] == ""} {
    set ::argv0 ""
}
if {[info var ::argv] == ""} {
    set ::argv ""
}

#
#  Call to require that the test colors be used
#
proc ReadoutGUIPanel::runInTestVersion {} {
    set ReadoutGUIPanel::UseTestColors 1
}
# ReadoutGUIPanel::addUserMenu ident label
#     Adds a user menu to the menubar of the
#     GUI. The caller is then responsible
#     for populating this menu with items (e.g. commands
#     separators checkboxes etc).
# Parameters:
#    ident   - Will be used as the last element of the widget
#              path for the new menu.
#    label   - The label of the menu on the menubar.
# Returns:
#    The full widget path for the new menu.
#
proc ReadoutGUIPanel::addUserMenu {ident label} {
    append menuname $::ReadoutGUIPanel::ROOT . $ident
    append menubar  $::ReadoutGUIPanel::ROOT . menu
    menu   $menuname
    $menubar add cascade -label $label -menu $menuname
    return $menuname

}
# ReadoutGUIPanel::addUserFrame ident
#    Adds a user frame to the bottom of the panel.
#    The user can place any widgets they please in this
#    frame laying it out as they desired.  The frame will
#    span the entire bottom of the gui.
#
# Parameters:
#  ident  - The last element of the wiget path of the frame
#           create.
# Returns:
#   The full path name of the created frame.
#
proc ReadoutGUIPanel::addUserFrame ident {
    set top $::ReadoutGUIPanel::ROOT
    append framename $top . $ident

    set geometry [grid size $top]
    set columns  [lindex $geometry 0]
    set rows     [lindex $geometry 1]
    incr rows

    frame $framename
    grid $framename -in $top -row $rows -columnspan $columns \
                    -sticky news

    return $framename
}


#
#  procs to set the callbacks for the run control buttons.
#
# ReadoutGUIPanel::startStopCallbacks startcb stopcb
#     Set the start/stop callbacks.
# Parameters:
#    startcb - new callback for start of run.
#    stopcb  - new callback for stop of run.
#
proc ReadoutGUIPanel::startStopCallbacks {startcb stopcb} {
    set ReadoutGUIPanel::beginCallback $startcb
    set ReadoutGUIPanel::endCallback   $stopcb
}
#     ReadoutGUIPanel::pauseResumeCallbacks pausecb resumecb
#
proc ReadoutGUIPanel::pauseResumeCallbacks {pausecb resumecb} {
    set ReadoutGUIPanel::pauseCallback  $pausecb
    set ReadoutGUIPanel::resumeCallback $resumecb
}
# ReadoutGUIPanel::setExitCallback callback
#     Set the exit callback.
# Parameters:
#    callback  -  callback script to call on exit.
#
proc ReadoutGUIPanel::setExitCallback {callback} {
    set ReadoutGUIPanel::exitCallback $callback
}
# ReadoutGUIPanel::setStartCallback  callback
#     Set a client callback to be invoked when the
#     user clicks the File->Start menu entry.
# Parameters:
#   callback   - The script to invoke.
#
proc ReadoutGUIPanel::setStartCallback callback {
    set ReadoutGUIPanel::startCallback $callback
}
# ReadoutGUIPanel::setRestartCallback callback
#    Set a client callback to be invoked when the
#    user clicks the File->Restart menu entry.
# Parameters:
#    callback  - The script to invoke.
#
proc ReadoutGUIPanel::setRestartCallback callback {
    set ReadoutGUIPanel::restartCallback $callback
}
# ReadoutGUIPanel::setSourceCallback callback
#    Sets a client callback for the source menu entry.
#
# Parameters:
#   callback  - The callback to invoke when the user
#               wants to source a file.
#
proc ReadoutGUIPanel::setSourceCallback callback {
    set ReadoutGUIPanel::sourceCallback $callback

}
# Lcal function to improve the contrast of a widget
# foreground when ghosted.
proc ReadoutGUIPanel::ImproveEntryContrast {widget} {
    # Not all widgets/versions of Tk support this.

    catch {$widget config -disabledforeground [$widget cget -foreground]}
}


# Local function to set a new state for a widget list.
#
proc ReadoutGUIPanel::SetWidgetListState {widgets state} {
    set base $::ReadoutGUIPanel::ROOT
    foreach widget $widgets {
        $base.$widget configure -state $state
        ReadoutGUIPanel::ImproveEntryContrast $base.$widget
    }
}

# ReadoutGUIPanel::ghostWidgets
#    Ghosts the widgets that should be disabled
#    on a start run.
#
proc ReadoutGUIPanel::ghostWidgets {} {
    ReadoutGUIPanel::SetWidgetListState \
            {runnumber title recording
             timed days hours minutes seconds} disabled
}
# ReadoutGUIPanel::ghostBegin
#    Ghost the start/stop button.
#
proc ReadoutGUIPanel::ghostBegin {} {
    ::ReadoutGUIPanel::SetWidgetListState startstop disabled

}
# ReadoutGUIPanel::unghostBegin
#    Enable the start/stop button.
#
proc ReadoutGUIPanel::unghostBegin {} {
    ::ReadoutGUIPanel::SetWidgetListState startstop normal
}
# ReadoutGUIPanel::unghostWidgets
#   Reactivate widgets that should be disabled on
#   a start run.
#
proc ReadoutGUIPanel::unghostWidgets {} {
    ReadoutGUIPanel::SetWidgetListState \
        {runnumber title recording
        timed days hours minutes seconds} normal
 }
# ReadoutGUIPanel::readoutRunning
#    enables Begin/End, Restart
#    widgets now that there's a Readout Program.
#
proc ReadoutGUIPanel::readoutRunning {} {
    set base $::ReadoutGUIPanel::ROOT
    append begin $base . startstop
    append filemenu $base . file

    $begin configure -state normal
    $filemenu entryconfigure Restart... -state normal
    $filemenu entryconfigure Start      -state normal
}
# ReadoutGUIPanel::readoutNotRunning
#    Ghosts begin/end, pause/resume,  Restart
#   widgets since there is no readout program
#   to manipulate.
proc ReadoutGUIPanel::readoutNotRunning {} {
    set base $::ReadoutGUIPanel::ROOT
    append begin $base . startstop
    append pause $base . pauseres
    append filemenu $base . file

    $begin configure -state disabled
    $pause configure -state disabled
    $filemenu entryconfigure Restart... -state disabled
}


# Retrieve stuff from texts:

# ReadoutGUIPanel::getHost
#    Retrieve the contents of the host entry widget.
#
proc ReadoutGUIPanel::getHost {} {
    set base $::ReadoutGUIPanel::ROOT
    append host $base . host
    return [$host cget -text]
}

# ReadoutGUIPanel::getPath
#   Get the path to the readout program in the
#   filesystem of the remote host.
#
proc ReadoutGUIPanel::getPath {} {
    set base $::ReadoutGUIPanel::ROOT
    append path $base . path
    return [$path cget -text]

}
#  ReadoutGUIPanel::setTitle  title
#      Set the new run title.
# Parameters:
#   title   - The run title.
#
proc ReadoutGUIPanel::setTitle title {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . title
    $w delete 0 end
    $w insert end $title
}
# ReadoutGUIPanel::getRunNumber
#    Returns the current requested run number.
#
proc ::ReadoutGUIPanel::getRunNumber {} {
    append widget $::ReadoutGUIPanel::ROOT . runnumber
    return [$widget get]
}
#   ReadoutGUIPanel::setRun     run
#       Set the run number entry.
# Parameters:
#     run   - The new run number.
#
proc ReadoutGUIPanel::setRun run {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . runnumber


    set oldState [::ReadoutGUIPanel::allowRunNumberEdit 1]


    $w configure -validate none
    $w delete 0 end
    $w insert end $run
    $w configure -validate key

    ::ReadoutGUIPanel::allowRunNumberEdit $oldState
    return
}
#
#  Disable/enable user modification of the run number widget
#  
#  @param flag - true for enable/false for disable.
#
#  @return boolean - prior value.
#
proc ReadoutGUIPanel::allowRunNumberEdit flag {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . runnumber
    set oldState  [$w cget -state]
    set newState [expr {$flag ? "normal" :  "disabled"}]
    $w configure -state $newState

    set result [expr {$oldState eq "normal" ? 1 : 0}]
    return $result


}
#  ReadoutGUIPanel::incrRun
#    Increments the run number.
#
proc ReadoutGUIPanel::incrRun {} {
    set run [::ReadoutGUIPanel::getRunNumber]
    incr run
    ::ReadoutGUIPanel::setRun $run
}
#   ReadoutGUIPanel::setHost    $host
#       Set the readout host value
# Parameters
#   host - The host on which the readout should run.
#
proc ReadoutGUIPanel::setHost host {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . host
    $w configure -text $host

#    append status $base . monitor
#    $status configure -host $host
}
#  ReadoutGUIPanel::setPath    $path
#     Set the readout path for the gui.
#
proc ReadoutGUIPanel::setPath path {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . path
    $w configure -text $path
}

# ReadoutGUIPanel::recordOff
#    Turns of the record data checkbox.
#
proc ReadoutGUIPanel::recordOff {} {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . recording
    $w deselect
    set ::ReadoutGUIPanel::RecordData 0
}
# ReadouGUIPanel::recordOn
#     Turn on the recording data checkbox.
proc ReadoutGUIPanel::recordOn {} {
    set base $::ReadoutGUIPanel::ROOT
    append w $base . recording
    $w select
    set ::ReadoutGUIPanel::RecordData 1
}
# ReadoutGUIPanel::setScalers channels period
#    Sets the scaler parameters.  This allows us
#    to preload the scaler dialog with the current
#    set of defaults.
# Parameters:
#   channels    - Number of scaler channels.
#   period      - Number of seconds between readouts.
#
proc ReadoutGUIPanel::setScalers {channels period} {
    set ::ReadoutGUIPanel::ScalerChannels $channels
    set ::ReadoutGUIPanel::ScalerPeriod   $period
}
# ReadoutGUIPanel::getScalers
#    Retrieves the current scaler parameters.
#    This can be any of the following:
#    - The default initial values if no other value
#      has been set and if the scaler menu has not
#      been invoked.
#    - An external setting made by setScalers if
#      the scaler menu entry has not been invoked since
#      the last setScalers call.
#    - The most recent results of the menu.
# Returns:
#    A 2 element list consisting of {channelcount period}
#
proc ReadoutGUIPanel::getScalers {} {
    return [list $::ReadoutGUIPanel::ScalerChannels \
                 $::ReadoutGUIPanel::ScalerPeriod]
}
##
# ReadoutGUIPanel::runIsStarting
#
#   Called ot indicate that a run is starting.
#  - Begin/End button is set to text Starting
#  - Border removed from the begin/end button to make it look
#    much more like a label
#  - Relief is set to flat as well.
#  These are all restored by runIsActive
#
proc ReadoutGUIPanel::runIsStarting {} {

    set root $::ReadoutGUIPanel::ROOT
    append startstop $root . startstop
    set ::ReadoutGUIPanel::buttonRelief [$startstop cget -relief]
    set ::ReadoutGUIPanel::buttonBorder [$startstop cget -borderwidth]

    $startstop configure -relief flat -borderwidth 0 -text {Starting..}
    update
    update
    update

}
##
# Similar to the above but indicates the run is ending.
#
proc ReadoutGUIPanel::runIsEnding {} {
    set root $::ReadoutGUIPanel::ROOT
    append startstop $root . startstop
    set ::ReadoutGUIPanel::buttonRelief [$startstop cget -relief]
    set ::ReadoutGUIPanel::buttonBorder [$startstop cget -borderwidth]

    $startstop configure -relief flat -borderwidth 0 -text {Ending...}
    update
    update
    update
}
#
#
# ReadoutGUIPanel::runIsActive
#    Called to indicate on the GUI that the run
#    is now active. The following actions are taken:
#    The startstop button is labelled End
#    The Pause/Resume button is unghosted and labelled Pause
#    ::ReadoutGUIPanel::ghostWidgets is called to disable
#    the appropriate set of widgets.
#
proc ReadoutGUIPanel::runIsActive {} {
    set root $::ReadoutGUIPanel::ROOT
    append startstop $root . startstop
    append pauseres  $root . pauseres

    $startstop configure -text End -relief $::ReadoutGUIPanel::buttonRelief \
	-borderwidth $::ReadoutGUIPanel::buttonBorder
    $pauseres  configure -text Pause -state normal 

    ::ReadoutGUIPanel::ghostWidgets

}
# ReadoutGUIPanel::runIsHalted
#     Called to set the widgets as they should be when
#     the run is completely inactive.
#
proc ReadoutGUIPanel::runIsHalted {} {
    set root $::ReadoutGUIPanel::ROOT
    append startstop $root .startstop
    append pauseres   $root .pauseres

    $startstop configure -text Begin -relief $::ReadoutGUIPanel::buttonRelief \
	-borderwidth $::ReadoutGUIPanel::buttonBorder
    $pauseres  configure -text Pause -state disabled

    ::ReadoutGUIPanel::unghostWidgets
}
# ReadoutGUIPanel::runIsPaused
#     Called to set widgets to the appropriate state
#     for a paused run.
#
proc ReadoutGUIPanel::runIsPaused {} {
    append pauseres $::ReadoutGUIPanel::ROOT . pauseres
    $pauseres configure -text Resume
}
# ReadoutGUIPanel::isRecording
#    Sets the background of the text and status line to
#    indicate event recording is on... a nice big
#    visible indicator... the background will be
#    spartan green.
#
proc ::ReadoutGUIPanel::isRecording {} {
    append text   $::ReadoutGUIPanel::ROOT . output
    append status $::ReadoutGUIPanel::ROOT . statusline

    if {$ReadoutGUIPanel::UseTestColors} {
	set bg $ReadoutGUIPanel::TestRecordingBg
	set fg $ReadoutGUIPanel::TestRecordingFg
    }    else {
	set bg {dark green}
	set fg {white}
    }
    puts "setting $fg $bg"
    $text configure -background $bg  -foreground $fg
    $status configure -background $bg -foreground $fg

}
# ReadoutGUIPanel::notRecording
#    Sets the background of the text and status line back to 'normal'.
#    Normal is defined as the current background of the toplevel widget
#    and the current foreground of the 'host label'.
#    All this 'cause I can't figure out how to read the foreground
#    value of labels etc. from the option database.
#
proc ::ReadoutGUIPanel::notRecording {} {
    append text   $::ReadoutGUIPanel::ROOT . output
    append status $::ReadoutGUIPanel::ROOT . statusline

    if {$ReadoutGUIPanel::UseTestColors} {
	set bg $ReadoutGUIPanel::TestNonRecordingBg
	set fg $ReadoutGUIPanel::TestNonRecordingFg
    }    else {
	set bg black
	set fg {dark green}
    }
    puts "setting $fg $bg"
    $text   config  -background $bg -foreground $fg
    $status config  -background $bg -foreground $fg
}

# ReadoutGUIPanel::normalColors
#    Resets the text and status areas to normal coloration.
#    (run is inactive).
#
proc ::ReadoutGUIPanel::normalColors {} {
    append text   $::ReadoutGUIPanel::ROOT . output
    append status $::ReadoutGUIPanel::ROOT . statusline
    append host   $::ReadoutGUIPanel::ROOT . host
    append title  $::ReadoutGUIPanel::ROOT . title



    if {$ReadoutGUIPanel::UseTestColors} {
	set bgcolor [$title cget -background]
	set fgcolor $ReadoutGUIPanel::TestRecordingFg
    } else {

	set bgcolor [$title cget -background]
	set fgcolor [$host              cget -foreground]
    }
    puts "setting normal"

    $text   config  -background $bgcolor -foreground $fgcolor
    $status config  -background $bgcolor -foreground $fgcolor
}

# ReadoutGUIPanel::recordData
#     True if the user requested data be recorded.
#
proc ::ReadoutGUIPanel::recordData {} {
    return $::ReadoutGUIPanel::RecordData
}
# ReadoutGUIPanel::outputText text
#   Outputs text to the output window.  The caller is responsible
#   for adding newlines at the appropriate places.
# Parameters:
#    text    - The text to add to the output window.
# NOTE:
#    In order to prevent this from being a memory leak, if the
#    number of lines of text in the text widget is bigger
#    than OutputSavedLines after the new text is appended,
#    the oldest lines are discarded so that the number of
#    lines in the text widget is about 10% of that
#
proc ReadoutGUIPanel::outputText text {
    append widget $::ReadoutGUIPanel::ROOT . output
    $widget configure -state normal
    $widget insert end $text
    set size [$widget index end]
    set lines [expr {int($size)}]
    if {$lines > $::ReadoutGUIPanel::OutputSavedLines} {
        set linestoleave [expr {int($::ReadoutGUIPanel::OutputSavedLines * 0.1)}]
	set linestokill [expr $lines - $linestoleave]
        $widget delete 0.0 $linestokill.0
    }
    $widget see end
    $widget configure -state disabled

}
##
# ReadoutGUIPanel::Log
#
#  Log text to the output window
#
#  @param src - log source (e.g. what is writing it).
#  @param sev - severity of the log 
#  @param msg - The message to log
#
proc RedougGUIPanel::Log {src sev msg} {
    set timestamp [clock format [clock seconds]]
    set msg "$timestamp : $src : $sev - $msg"
    outputText $msg

}
# ReadoutGUIPanel::setActiveTime days hours minutes seconds
#     Sets the contents of the elapsed active run time label.
#     This will contain a representation of the time for  which
#     the run has actually been taking data.
# Parameters:
#   stamp  - The time you want to appear on the time widget.
#
proc ::ReadoutGUIPanel::setActiveTime {stamp} {
    append widget $::ReadoutGUIPanel::ROOT . elapsed
    $widget configure -text $stamp
}
# ReadoutGUIPanel::setStatusLine line
#     puts a new text string in the status line label.
# Parameters:
#    line - New contents of the status line label.
#
proc ::ReadoutGUIPanel::setStatusLine line {
    append widget $::ReadoutGUIPanel::ROOT . statusline
    $widget configure -text $line
}

# ReadoutGUIPanel::getTitle
#    Returns the current title string.
#
proc ::ReadoutGUIPanel::getTitle {} {
    append widget $::ReadoutGUIPanel::ROOT . title
    return [$widget get]
}

# ReadoutGUIPanel::isTimed
#    nonzero if the timed checkbox is selected indicating
#    the run should be a timed run.
#
proc ReadoutGUIPanel::isTimed {} {
    return $::ReadoutGUIPanel::TimedRun
}
# ReadoutGUIPanel::setTimed state
#     Set state of timed run.
#
proc ReadoutGUIPanel::setTimed {state} {
    set ::ReadoutGUIPanel::TimedRun $state
    append widget $::ReadoutGUIPanel::ROOT . timed
    if {$state} {
        .timed select
    } else {
        .timed deselect
    }
}
# ReadoutGUIPanel::getRequestedRunTime
#   Find out how long the user requested the
#   run to be.  This is done by reading the
#   days, hours, minutes and seconds widgets
#   and computing what that means in seconds.
#
proc ReadoutGUIPanel::getRequestedRunTime {} {
    append wdays    $::ReadoutGUIPanel::ROOT . days
    append whours   $::ReadoutGUIPanel::ROOT . hours
    append wminutes $::ReadoutGUIPanel::ROOT . minutes
    append wseconds $::ReadoutGUIPanel::ROOT . seconds

    set elapsed [$wdays get];            # Days.
    set elapsed [expr {$elapsed*24 + [$whours   get]}]; #Hours
    set elapsed [expr {$elapsed*60 + [$wminutes get]}]; # Minutes
    set elapsed [expr {$elapsed*60 + [$wseconds get]}]; # seconds.

    return $elapsed
}
# ReadoutGUIPanel::setRequestedRunTime time
#       Set the requested run time in seconds.
#
proc ReadoutGUIPanel::setRequestedRunTime {time} {
    append wdays    $::ReadoutGUIPanel::ROOT . days
    append whours   $::ReadoutGUIPanel::ROOT . hours
    append wminutes $::ReadoutGUIPanel::ROOT . minutes
    append wseconds $::ReadoutGUIPanel::ROOT . seconds

    set secs [expr $time % 60]
    $wseconds set $secs

    set time [expr $time/60]
    set min  [expr $time % 60]
    $wminutes set $min

    set time  [expr $time /60]
    set hours [expr $time % 60]
    $whours set $hours

    set days   [expr $time / 24]
    $wdays set $days
}
# END USER CODE

# BEGIN CALLBACK CODE
# ONLY EDIT CODE INSIDE THE PROCS.

# ReadoutGUIPanel::days_command --
#
# Callback to handle days widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::days_command args {}

# ReadoutGUIPanel::days_invalidcommand --
#
# Callback to handle days widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::days_invalidcommand args {}

# ReadoutGUIPanel::days_validatecommand --
#
# Callback to handle days widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::days_validatecommand args {}

# ReadoutGUIPanel::days_xscrollcommand --
#
# Callback to handle days widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::days_xscrollcommand args {}

# ReadoutGUIPanel::host_invalidcommand --
#
# Callback to handle host widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::host_invalidcommand args {}

# ReadoutGUIPanel::host_validatecommand --
#
# Callback to handle host widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::host_validatecommand args {}

# ReadoutGUIPanel::host_xscrollcommand --
#
# Callback to handle host widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::host_xscrollcommand args {}

# ReadoutGUIPanel::hours_command --
#
# Callback to handle hours widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::hours_command args {}

# ReadoutGUIPanel::hours_invalidcommand --
#
# Callback to handle hours widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::hours_invalidcommand args {}

# ReadoutGUIPanel::hours_validatecommand --
#
# Callback to handle hours widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::hours_validatecommand args {}

# ReadoutGUIPanel::hours_xscrollcommand --
#
# Callback to handle hours widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::hours_xscrollcommand args {}

# ReadoutGUIPanel::minutes_command --
#
# Callback to handle minutes widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::minutes_command args {}

# ReadoutGUIPanel::minutes_invalidcommand --
#
# Callback to handle minutes widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::minutes_invalidcommand args {}

# ReadoutGUIPanel::minutes_validatecommand --
#
# Callback to handle minutes widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::minutes_validatecommand args {}

# ReadoutGUIPanel::minutes_xscrollcommand --
#
# Callback to handle minutes widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::minutes_xscrollcommand args {}

# ReadoutGUIPanel::output_xscrollcommand --
#
# Callback to handle output widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::output_xscrollcommand args {}

# ReadoutGUIPanel::path_invalidcommand --
#
# Callback to handle path widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::path_invalidcommand args {}

# ReadoutGUIPanel::path_validatecommand --
#
# Callback to handle path widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::path_validatecommand args {}

# ReadoutGUIPanel::path_xscrollcommand --
#
# Callback to handle path widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::path_xscrollcommand args {}

# ReadoutGUIPanel::pauseres_command --
#
# Callback to handle pauseres widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::pauseres_command args {
    append button $::ReadoutGUIPanel::ROOT . pauseres
    if {[$button cget -text] == "Pause"} {
        if {[info commands $::ReadoutGUIPanel::pauseCallback] != ""} {
            $::ReadoutGUIPanel::pauseCallback
        }
    } else {
        if {[info commands $::ReadoutGUIPanel::resumeCallback] != ""} {
            $::ReadoutGUIPanel::resumeCallback
        }
    }
}

# ReadoutGUIPanel::recording_command --
#
# Callback to handle recording widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::recording_command args {
    set ::ReadoutGUIPanel::RecordData \
        [expr $::ReadoutGUIPanel::RecordData ? 0 : 1]
}

# ReadoutGUIPanel::runnumber_invalidcommand --
#
# Callback to handle runnumber widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::runnumber_invalidcommand args {
    bell
}

# ReadoutGUIPanel::runnumber_xscrollcommand --
#
# Callback to handle runnumber widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::runnumber_xscrollcommand args {}

# ReadoutGUIPanel::seconds_command --
#
# Callback to handle seconds widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::seconds_command args {}

# ReadoutGUIPanel::seconds_invalidcommand --
#
# Callback to handle seconds widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::seconds_invalidcommand args {}

# ReadoutGUIPanel::seconds_validatecommand --
#
# Callback to handle seconds widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::seconds_validatecommand args {}

# ReadoutGUIPanel::seconds_xscrollcommand --
#
# Callback to handle seconds widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::seconds_xscrollcommand args {}

# ReadoutGUIPanel::startstop_command --
#
# Callback to handle startstop widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::startstop_command args {
    #  The callback invoked is
    # dependent on the text in the widget.

    append button $::ReadoutGUIPanel::ROOT . startstop
    if {[$button cget -text] == "Begin"} {
        if {[info commands $::ReadoutGUIPanel::beginCallback] != ""} {
            $::ReadoutGUIPanel::beginCallback
        }
    } else {
        if {[info commands $::ReadoutGUIPanel::endCallback] != ""} {
            $::ReadoutGUIPanel::endCallback
        }
    }
}

# ReadoutGUIPanel::timed_command --
#
# Callback to handle timed widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::timed_command args {
    set ::ReadoutGUIPanel::TimedRun [expr {$::ReadoutGUIPanel::TimedRun ? 0 : 1}]
}

# ReadoutGUIPanel::title_invalidcommand --
#
# Callback to handle title widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::title_invalidcommand args {
    bell
}

# ReadoutGUIPanel::title_validatecommand --
#
# Callback to handle title widget option -validatecommand
# Require the data in the title string be 80 characters long at most.
#
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::title_validatecommand args {
    set widget [lindex $args 0]
    set type [lindex $args 1]
    if {$type == 1} {
	set string [$widget get]
	if {[string length $string] == 80} {
	    return 0
	}
    
    }
    return 1
}

# ReadoutGUIPanel::title_xscrollcommand --
#
# Callback to handle title widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::title_xscrollcommand args {}

# ReadoutGUIPanel::_entry_1_invalidcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_entry_1_invalidcommand args {}

# ReadoutGUIPanel::_entry_1_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_entry_1_validatecommand args {}

# ReadoutGUIPanel::_entry_1_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_entry_1_xscrollcommand args {}

# ReadoutGUIPanel::_entry_4_invalidcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_entry_4_invalidcommand args {}

# ReadoutGUIPanel::_entry_4_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_entry_4_validatecommand args {}

# ReadoutGUIPanel::_entry_4_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_entry_4_xscrollcommand args {}

# ReadoutGUIPanel::_spinbox_5_command --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_spinbox_5_command args {}

# ReadoutGUIPanel::_spinbox_5_invalidcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_spinbox_5_invalidcommand args {}

# ReadoutGUIPanel::_spinbox_5_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_spinbox_5_validatecommand args {}

# ReadoutGUIPanel::_spinbox_5_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_spinbox_5_xscrollcommand args {}

# ReadoutGUIPanel::_text_1_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_text_1_xscrollcommand args {}

# ReadoutGUIPanel::_text_2_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::_text_2_xscrollcommand args {}

# ReadoutGUIPanel::record_command --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::record_command args {}

# ReadoutGUIPanel::runnumber_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::runnumber_validatecommand args {

    set    text   $args
    if {[regexp "(^\[0-9\]+\$)" $text]} {
	return 1
    } else {
	return 0
    }

}
# ReadoutGUIPanel::scalerparameters_command --
# Invoked by a click onthe Scaler->parameters... menu.
#
# ARGS:
#    <NONE>
#
proc ReadoutGUIPanel::scalerparameters_command args {
    set newinfo [ScalerParameterGUI::modal .scalerparameters \
                            $ReadoutGUIPanel::ScalerChannels \
                            $ReadoutGUIPanel::ScalerPeriod ]
    set ReadoutGUIPanel::ScalerChannels [lindex $newinfo 0]
    set ReadoutGUIPanel::ScalerPeriod   [lindex $newinfo 1]
}
# ReadoutGUIPanel::filenew_command --
#    Invoked by a click on the File->new... menu.
#  Accepts potentially new values for the
#  host/filename.
#
proc ReadoutGUIPanel::filenew_command args {
    set newinfo [selectReadoutDialog::createModal .readoutprompt \
                    [ReadoutGUIPanel::getHost] [ReadoutGUIPanel::getPath]]
    ReadoutGUIPanel::setHost [lindex $newinfo 0]
    ReadoutGUIPanel::setPath [lindex $newinfo 1]
}

# ReadoutGUIPanel::fileexit_command
#     Called when the File->Exit is hit.
#     We confirm the user wants to exit.. If so, the
#     Client's exit callback (if established) is called.
#     and then we exit.
#
proc ReadoutGUIPanel::fileexit_command args {
    if {[tk_dialog .confirmexit "Exit confirmation" \
            {Are you sure you want to exit the GUI and readout program?} \
            warning 1 Yes No] == 0} {

        if {$ReadoutGUIPanel::exitCallback != ""} {
            eval $ReadoutGUIPanel::exitCallback
        }
        exit
    }
}
# ReadoutGUIPanel::filestart_command
#    Called by the File->Start command.
#    If the user has a callback established, we call it.
#
proc ReadoutGUIPanel::filestart_command args {
    if {$ReadoutGUIPanel::startCallback != ""} {
        eval $ReadoutGUIPanel::startCallback
    }
}
# ReadoutGUIPanel::filerestart_command
#     Called in response to the File->Restart menu entry.
#
proc ReadoutGUIPanel::filerestart_command args {
    if {$ReadoutGUIPanel::restartCallback != "" } {
        eval $ReadoutGUIPanel::restartCallback
    }
}
# ReadoutGUIPanel::filesource_command
#     Called in response to the File->Source menu entry.
#
proc ReadoutGUIPanel::filesource_command args {
    set file [tk_getOpenFile -initialdir ReadoutGUIPanel::sourceDefaultPath \
              -defaultextension .tcl \
              -filetypes {
                 {{TCL Scripts} {.tcl}           }
                 {{TK scripts}  {.tk}            }
                 {{All files}    *               }
               }
             ]
    # If the user handed us a file, invoke our callback to the app.

    if {($file != "") &&($ReadoutGUIPanel::sourceCallback != "")} {
        $ReadoutGUIPanel::sourceCallback $file
    }
}
# END CALLBACK CODE

# ReadoutGUIPanel::init --
#
#   Call the optional userinit and initialize the dialog.
#   DO NOT EDIT THIS PROCEDURE.
#
# Arguments:
#   root   the root window to load this dialog into
#
# Results:
#   dialog will be created, or a background error will be thrown
#
proc ReadoutGUIPanel::init {root args} {
    # Catch this in case the user didn't define it
    catch {ReadoutGUIPanel::userinit}
    if {[info exists embed_args]} {
	# we are running in the plugin
	ReadoutGUIPanel::ui $root
    } elseif {$::argv0 == [info script]} {
	# we are running in stand-alone mode
	wm title $root ReadoutGUIPanel
	if {[catch {
	    # Create the UI
	    ReadoutGUIPanel::ui  $root
	} err]} {
	    bgerror $err ; exit 1
	}
    }
    if {$root == "."} {
        set ReadoutGUIPanel::ROOT {}
    }
    catch {ReadoutGUIPanel::run $root}
}



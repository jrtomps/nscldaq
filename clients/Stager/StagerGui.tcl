#!/bin/sh
# Start Wish. \
exec wish ${0} ${@}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
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

tk_messageBox -icon info -title "Not Used" \
    -message {The stager program is no longer in use at the NSCL you should exit this}



# Set up the auto_path to include
# the full daq directory tree.
#
set here [file dirname [info script]]
set libdir [file join $here ..]
#
# Canonicalize $here/..
#
set wd [pwd]
cd $libdir
set libdir [pwd]
cd $wd
#   
#  Prepend to auto_path only if it's not already 
#  there.
#
if {[lsearch -exact $auto_path $libdir] == -1} {
    set auto_path [concat $libdir $auto_path]
}



package require Stager
package require GetTape
package require SelectStagePolicy
package require StagePolicyUsed
package require StagePolicyFree
package require ExpStagePolicy
package require KeeplistGUI
package require RunRetension
package require ExpFileSystem
package require ReadoutGui
package require ReadoutControl
package require Experiment
package require ExpFileSystem
package require UniqueInstance
package require Wait
#
#   Functional procs.
#
#
#  Stage.  Called whenever it is time to stage
#          This can be either as a result of periodic polling of the
#          Stage strategy or as a result of manual staging.
#
set SourceHost ""
set ReadoutPath ""
set ftphost ""
set tapehost "";			# This will default to the ftp host.
set Password ""
set TapeCapacity Unknown
set TapeMounted  0
set TapeUsed     0
set TapeNumber   1
set TarFileNum   1
set TapeLogFile [ExpFileSystem::WhereisMetadata]/TapeIndex.txt
set StageStrategy "Not Set"
set TapeDrive   "Not Set"
set configfilename ".stagerconfig"
set hashkey        "nscldaq"
set stagelist      ""               ;# Only used on state restore.

proc NonModalConfirm {top title text OkValue CancelValue} {
    global NonModalResult
    
    # build the dialog widgets;  a top frame with the questhead
    # bitmask and the question text.
    # A bottom frame with ok and cancel buttons.
    #
    toplevel $top
    
    wm title $top $title
    set query  [frame $top.query]
    set action [frame $top.action]
    label $query.quest -bitmap questhead 
    label $query.text  -text $text -wraplength 2i
    pack $query $action
    pack $query.quest $query.text -side left

    button $action.ok -text "  Ok  " -command "set NonModalResult $OkValue"
    button $action.cancel -text "Cancel" \
	    -command "set NonModalResult $CancelValue"
    pack $action.ok $action.cancel -side left -anchor c

    vwait NonModalResult
    destroy $top
    return $NonModalResult

}
#
#  Confirm exit:
#
proc ConfirmExit {} {
    set answer [tk_dialog .confirm "Exit?" \
	    "Are you sure you want to exit the stager?" \
	    questhead 1 Ok Cancel]
    if {$answer == 0} {
	UniqueInstance::Exit Stager ConfirmExit
    }
}
#
#   Put data into text window:
#
proc PutTextToWindow {text} {
    .tarout.output config -state normal
    .tarout.output insert end $text
    .tarout.output config -state disabled
    .tarout.output see end
    update idletasks
    update idletasks
    update idletasks
}
proc ClearTextWindow {} {
    .tarout.output config -state normal
    .tarout.output delete 0.0 end
    .tarout.output config -state disabled
    update idle
    update idle
    update idle
}
#
#  Simple hash of password:
#
proc HashPassword {pwd} {
    global hashkey
    
    # convert hash and password to binary arrays of bytes.
    

     set hashlength [string length $hashkey]
    for {set i 0} {$i < $hashlength} {incr i} {
	binary scan  [string index $hashkey $i] c1 hash($i)
    }
    set pwdlength [string length $pwd]
    for {set i 0} {$i < $pwdlength} {incr i} {
	binary scan [string index $pwd $i] c1 pass($i)

    }
    # hash and output byte by byte:
    #
    set hashidx 0
    for {set i 0} {$i < $pwdlength} {incr i} {
	lappend hashedbytes [expr $pass($i) ^ $hash($hashidx)]
	incr hashidx
	if {$hashidx >= $hashlength} {set hashidx 0}
    }
    return $hashedbytes
    
}
proc UnhashPassword {hashedpw} {
    global hashkey

    # Convert hash key to list of bytes:

    set hashlength [string length $hashkey]
    for {set i 0} {$i < $hashlength } {incr i} {
	binary scan [string index $hashkey $i] c1 hash($i)
    }
    #  Hash the password with the hash array:

    set hashidx 0
    foreach byte $hashedpw {
	lappend unhashedpw [expr $byte ^ $hash($hashidx)]
	incr hashidx
	if {$hashidx >= $hashlength} {set hashidx 0}
    }

    # Convert the rehashed password back to a string:

    foreach byte $unhashedpw {
	set octal [format "%o" $byte]
	eval set char "\\$octal"
	append passwd $char
    }
    return $passwd
}
#
#  Save the current program settings to the file
#    Root/.stagerconfig
proc SaveSettings {} {
    global SourceHost
    global ReadoutPath
    global ftphost
    global tapehost
    global Password
    global TapeCapacity
    global TapeNumber
    global StageStrategy
    global TapeDrive
    global configfilename

    set file [ExpFileSystem::WhereisMetadata]
    append file "/" $configfilename
    set fd [open $file "w"]

    # Save the items which can be restored via simple set var value:

    foreach var {SourceHost ReadoutPath ftphost tapehost 
	TapeCapacity TapeNumber StageStrategy TapeDrive} {
	  eval set value $$var
	  puts $fd "set $var $value"
    }
    #  The stage threshold is gotten from the stagepolicy module:
    #
    puts $fd "set StageThreshold [ExpStagePolicy::GetThreshold]"

    # The password is hashed:

    puts $fd "set HashedPassword {[HashPassword $Password]}"

    # The retension list is saved:
    
    set  stagelist [ExpRunRetension::ListPending]
    puts $fd "set stagelist {$stagelist}"


    # done

    close $fd 
    
}
#
#  Read settings from file
#
proc ReadSettings {name} {
    global SourceHost
    global ReadoutPath
    global ftphost
    global tapehost
    global Password
    global TapeCapacity
    global StageStrategy
    global TapeDrive
    global stagelist
    global TapeNumber

    source $name

    # The globals will have been set. This leaves only:
    #    StageThreshold: - The staging threshold
    #    HashedPassword  - The hased password.  

   # The hashed password is unhashed:

    set Password [UnhashPassword $HashedPassword]

    #  The retension list is restored:

    foreach dir $stagelist {
	ExpRunRetension::AddToPending $dir
    }

    # The StageThreshold is just returned to the caller:

    return $StageThreshold

}
#   Set stage policy given name:  Stager is initialized too:
#
proc StartStager {policyname} {
    set cmd $policyname
    append cmd ::Register
    eval $cmd
    Stager::Initialize

}
#
#  Start readout program:
#
#
proc StartReadout {SourceHost ReadoutPath ftphost Password} {
    global env

    set Path $env(PATH)
    set Path [split $Path " :"]
    set TclLibPath $env(TCLLIBPATH)
    foreach element $TclLibPath {lappend Path $element}
    set found 0
    foreach directory $Path {
	set program [glob -nocomplain  $directory/ReadoutShell.tcl]
	if {$program != ""} {
	    set found 1
	    exec wish $program $SourceHost $ReadoutPath $ftphost $Password &
	    break;
	}
    }
    update idletasks; update idletasks; update idletasks
    if {!$found} {
#	puts "Could not find ReadoutShell.tcl in any of $Path"
	exit
    }
}
#   Splits a stage operation in to a set of tapes.
#   Input are the paths to the directories to stage.
#   output is a list where each item contains:
#
#     {File_list Gigabytes_Required}
#
#        File_list is itself a list of the directories to fit on that
#                  tape.
#        Gigabytes_Required is the space required by each tape.
#  Implicit inputs are:
#     TapeCapacity - the size of a tape.
#     TapeUsed     - Gbytes used on current tape.
#  Method:
#     Files are sequentially put into a tape until they don't fit.
#     No attempt is made to reorder runs as this will violate
#     expectations.
#
proc SplitIntoTapes {files} {
    global TapeCapacity
    global TapeUsed
    set thistape "";			;# Initialize list for this tape empty.
    set capacity [expr $TapeCapacity*(1.0 - 0.25)]   ;# give 25% for compres slop.

    #  Build a list of run sizes
    #
    foreach file $files {
	lappend runSizes [expr [OS::DiskSpaceWLinks $file]/(1024.0*1024.0)]
    }
    set Remaining [expr $capacity - $TapeUsed]
    set tapes    ""
    set index    0
    set thissize 0.0
    set Result   ""

    foreach file $files {
	if {[lindex $runSizes $index] > $Remaining ||
            ($index == [llength $files])} {
	    # Close off a tape - Add the list entry describing
	    # this tape segment to Result
	    # and set up for the next tape.

	    lappend Result [list $thistape $thissize]
	    
	    set thissize 0.0
	    set Remaining $TapeCapacity
            set thistape ""
	}
	#  Append file to this tape... count down the
	#  remaining size and up the size of this tape segment.
	#
	lappend thistape $file
	set    thissize [expr $thissize + [lindex $runSizes $index]]
	set    Remaining [expr $Remaining - [lindex $runSizes $index]]

	incr index
    }

    if {[llength $thistape] != 0}  {
	lappend Result [list $thistape $thissize]
    }


    return $Result
    
}
#
#  Procedure to log the successful staging of files to tape:
#
proc LogStage {Files} {
    global TapeLogFile
    global TapeNumber
    global TarFileNum
    global TapeMounted

    set fd [open $TapeLogFile "a"]
    set time [clock format [clock seconds]]
    puts $fd \
	 "---------------------- $time Stage pass performed -----------------"
    foreach file $Files {
	puts $fd \
	"$time $file Staged to Tape $TapeNumber in tar file# $TarFileNum"
    }
    flush $fd
    close $fd
    
}
#
#  Procedure to write a tape worth of data.
#
proc StageATape {StageFiles} {

    return [Stager::WriteToTape $StageFiles ::PutTextToWindow]

}
#
#  Procedure to oversee Staging.
#
proc Stage {} {
    global TapeUsed
    global TapeNumber
    global TapeMounted
    global TarFileNum

    set oldcursor [. cget -cursor]
    update idletasks
    update idletasks

    ClearTextWindow
    
    set StageFiles [Stager::GenerateStageList]
    if {[llength $StageFiles] == 0} {
	set message "Stager found nothing to stage.\n"
	append message "You may need to clean up  the stage area manually\n"
	append message "and re-set the staging threshold."
	Diagnostics::Info $message
	.levels.threshold set      [lindex [ExpStagePolicy::GetLimits] 1]
    } else {
	set TapeList [SplitIntoTapes $StageFiles]
	if {([llength [lindex [lindex $TapeList 0] 0]] == 0)} {
	    set TapeMounted 0
	    incr TapeNumber
	    set TarFileNum 1
	}

	set tapeinset 1
	set tapestodo [llength $TapeList]
	foreach Tape $TapeList {
	    set Files [lindex $Tape 0]
	    set Size  [lindex $Tape 1]

	    if {$TapeMounted == 0} {
	    tk_dialog .replace "replace tape" \
		"Please remove tape from drive replace with tape $TapeNumber" \
		warning 0 "Proceed"
		set TapeMounted 1
	    }
	    . config -cursor watch

	    PutTextToWindow \
             "Staging $Size Gbytes of data to tape #  $TapeNumber\n"
	    set status [StageATape $Files]
	    update idletasks
	    update idletasks
	    update idletasks
	    update idletasks              ;# Ensure tar output is complete.
	    update idletasks

	    . config -cursor $oldcursor

	    set message "Tape $tapeinset of $tapestodo written\n"
	    if {$status != 0 } {
		set emsg $status
		append message \
                     "Note: tar returned abnormal status: $emsg \n"
	    }
	    append message \
		"If the tar of the data succeeded, click Ok, else Cancel";
	    .buttons.stage config -state disabled
	    set answer [NonModalConfirm .confirm "Confirmation" \
		    $message \
		   0  1 ]
	    .buttons.stage config -state normal
	    if {$answer == 0} {
		LogStage $Files
		set Retained [Stager::DeleteEventData $Files]
#		puts "Retaining $Retained"
		Stager::MoveRetainedData $Retained
		Stager::MoveMetaData $Files
	    } else {
		Diagnostics::Info "Stager cleanup of event data refused"
		break
	    }
	    incr TarFileNum
	    set TapeMounted 0
	    set TapeUsed $Size
	    incr tapeinset
	}
	set TapeMounted 1
    }
    . config -cursor $oldcursor
}
#
#  StageTimer - Called to periodically check stage criteria to 
#               see if it's time to stage.
#
proc StageTimer {period} {
    SaveSettings
    if {[ExpStagePolicy::StageNow]} {
	Stage
    }
    
    #  Reschedule self.
    
    after $period StageTimer $period
}
#
#  SetThreshold - set a new threshold value.
#
proc SetThreshold {value} {
    ExpStagePolicy::SetThreshold $value
}
#
#  MaintainCapacity widget 
#    Keep the value of the capacity widget up to date with second resolution.
#
#  Note; set has no effect on disabled scales so we need to activate and
#        re-disable.

proc MaintainCapacity {wid} {
    $wid config -state active
    $wid set [ExpStagePolicy::StageCapacity]
    $wid config -state disabled
    after 1000 MaintainCapacity $wid
}  
proc NonLink {mask} {
    set files [glob -nocomplain $mask]
    set nonlinks ""
    foreach file $files {
	if {[catch {file readlink $file}] != 0} {
	    lappend nonlinks $file
	}
    }
    return $nonlinks
}
proc ChangeTape {} {
    global TapeMounted
    global TapeNumber
    global TapeUsed
    global TarFileNum

    incr TapeNumber
    tk_dialog .changetape "Change tape"\
       "Please Remove the current tape and replace with tape $TapeNumber" \
       warning 0 "Proceed"
    set TapeMounted 1
    set TapeUsed    0
    set TarFileNum  0

}
#
#  Manage retained runs:
#
proc RetainDlg {} {
    # Get the current run list, the pending and retained lists.
    #
    set pending  [ExpRunRetension::ListPending]
    set retained [ExpRunRetension::ListRetained]
    set current  [NonLink [ExpFileSystem::WhereisMetadata]/run*]
    set currentnotpending ""
    foreach file $current {
	if {![ExpRunRetension::isPending $file]} {
	    lappend currentnotpending $file
	}
    }
    
    set NewPending \
	    [KeeplistGUI::KeeplistDialog $currentnotpending $pending $retained]
    #
    #  Add new items and remove the removed ones:
    #
    foreach item $NewPending {
	if {[lsearch -exact $pending $item] == -1} {
	    ExpRunRetension::AddToPending $item
	}
    }
    foreach item $pending {
	if {[lsearch -exact $NewPending $item] == -1} {
	    ExpRunRetension::RemoveFromPending $item
	}
    }
}


proc MainGui {} {
    global TapeNumber
    global TarFileNum


    SaveSettings

    #  Setup the GUI in the main window.

    wm deiconify .
    #
    #  Figure out how to lay out the scales.
    #
    set limits [ExpStagePolicy::GetLimits]
    set low [lindex $limits 0]
    set hi  [lindex $limits 1]
    set mid [expr ($hi - $low)/2.0]
    set tickinterval [expr ($hi-$low)/5.0]
    set units [ExpStagePolicy::GetUnits]
    #
    # Frame with stage level indicators.
    #
    frame .levels
    scale .levels.threshold -from $low -to $hi -bigincrement $tickinterval \
	    -label "Stage threshold in $units" -showvalue 1  -length 3i \
	    -sliderrelief sunken  -resolution 0.01\
	    -tickinterval $tickinterval -orient horizontal \
	    -command SetThreshold
    scale .levels.current -from $low -to $hi -bigincrement $tickinterval \
	    -label "Current usage level in $units" -showvalue 1 -length 3i \
	    -sliderrelief sunken  -resolution 0.01\
	    -tickinterval $tickinterval -orient horizontal -state disabled
    set tvalue [ExpStagePolicy::GetThreshold]
    .levels.threshold set $tvalue
    pack .levels -side top
    pack .levels.threshold .levels.current -side top
    #
    # buttons.
    #
    frame .buttons
    button .buttons.exit -text Exit -command ConfirmExit
    button .buttons.stage -text "Stage Now" -command Stage
    button .buttons.retain -text "Retain..."  -command RetainDlg
    #
    # Tar outut and error:
    #
    frame .tarout
    text  .tarout.output -width 70 -state disabled \
	    -yscrollcommand ".tarout.scrollbar set"
    scrollbar .tarout.scrollbar -orient vertical \
	    -command ".tarout.output yview"

    frame  .tapestatus
    label  .tapestatus.tapet      -text "Tape number: "
    label  .tapestatus.tapenumber -textvariable TapeNumber
    label  .tapestatus.filet      -text " File number: "
    label  .tapestatus.filenumber -textvariable TarFileNum
    label  .tapestatus.used       -textvariable TapeUsed
    label  .tapestatus.usedt      -text "Gbytes written"
    button .tapechange            -text "Change Tape" -command ChangeTape

    
    pack .buttons -side top
    pack .buttons.exit .buttons.stage .buttons.retain -side left
    pack .tarout
    pack .tarout.output -side left -fill x
    pack .tarout.scrollbar -side right -fill y
    pack .tapestatus -side top
    pack .tapestatus.tapet .tapestatus.tapenumber .tapestatus.filet \
	 .tapestatus.filenumber .tapestatus.used .tapestatus.usedt \
	 -side left -fill x
    pack .tapechange -side top

    StageTimer  5000
    MaintainCapacity .levels.current

}

#
#  Got the staging policy selected, register it
#  and initialize the stager before setting up the main gui.
#
proc PolicyGotten {Policy} {
    global   StageStrategy
    global   SourceHost
    global   ReadoutPath
    global   Password
    global   env

#    puts "In Policy"
    StartStager $Policy
    bind    .getpolicy <Destroy> {}
#    puts "Destroy for .getpolicy rebound to empty"
    destroy .getpolicy
#    puts ".getpolicy destroyed"

    set StageStrategy   $Policy
    MainGui
    
}
#
#  Got the readout info.  Save it and get the staging policy.
#
proc ReadoutGotten {toplevel host path} {
    global  SourceHost
    global ReadoutPath
    global ftphost
    global Password

    set SourceHost $host
    set ReadoutPath $path

    ReadoutControl::SetReadoutProgram $host $path


    Experiment::Register
    bind    $toplevel <Destroy> {}
    destroy $toplevel
    toplevel .getpolicy
    bind .getpolicy <Destroy> "UniqueInstance::Exit Stager ReadoutGotten"

    wm title .getpolicy "Select stage policy"
    SelectStagePolicy::SetStagePolicy .getpolicy PolicyGotten \
	    {{Used space on Stage disk >  Threshold} 
             {Event Data Size Threshold}} \
	    {ExpStagePolicyFree ExpStagePolicyUsed}
    StartReadout $SourceHost $ReadoutPath $ftphost $Password
}
#
#  Called when we have the tapedrive and the staging host.  Now we go
#  for the front end host and path to Readout relative to ~/experiment/current
#
proc GetReadoutspec {} {
    toplevel .readoutdialog
    bind .readoutdialog <Destroy> "UniqueInstance::Exit Stager GetReadoutspec"

    wm title .readoutdialog "Select Readout and host"
    ReadoutGui::GetReadoutSpec .readoutdialog ReadoutGotten
    
}
#
#  Called when we have a valid tapedrive and host.
#  set it in the stager and setup the stage policy selection gui.
#
proc TapeGotten {host ftp password drive capacity} {
    global tapehost
    global ftphost
    global Password
    global TapeCapacity
    global TapeDrive

    set Password $password
    set tapehost  $host
    set ftphost   $ftp
    set TapeDrive $drive

    puts "Tapehost = $tapehost"
    puts "drive    = $TapeDrive"
    puts "FTPhost  = $ftphost"


    bind    .getdevice <Destroy> {}
    destroy .getdevice
    Stager::SetHost  $host
    Stager::SetDrive $drive
    set TapeCapacity $capacity

    Experiment::SetFtpLogInfo $host t$password
    GetReadoutspec

}
#
#   Get the settings from the gui.
#
proc GuiGetSettings {} {
    #
    #  Init to select tape drive.
    #
    toplevel .getdevice
    bind .getdevice <Destroy> "UniqueInstance::Exit Stager GuiGetSettings"

    wm title .getdevice "Select Taping host and drive."
    GetTape::GetTapeDrive .getdevice TapeGotten
}
ExpFileSystem::CreateHierarchy

#
#  Confirm settings are ok... if not go to gui.
#
proc GuiConfirmSettings {threshold} {
    global ftphost
    global tapehost
    global Password
    global SourceHost
    global ReadoutPath
    global TapeCapacity
    global TapeDrive
    global StageStrategy
    global TapeNumber

    # If tapehost is null then we have an old
    # config file and need to re-prompt.

    if {$tapehost == ""} {
	GuiGetSettings
	return;	
    }
    #  Otherwise list the settings:
    #  and ask for confirmation.
    #

    toplevel .settings 
    bind .settings <Destroy> "UniqueInstance::Exit Stager GuiConfirmSettings"

    wm title .settings "Confirm Settings"
    frame    .settings.top
    frame    .settings.bottom -relief groove -borderwidth 3

    set labellist { 
	{"FTP destination :"  $ftphost}
	{"Tape drive host :"  $tapehost}
	{"Staging Tape   :"  $TapeDrive}
	{"Tape Capacity   :"  $TapeCapacity}
	{"Stage Method    :"  $StageStrategy}
	{"      Threshold :"  $threshold}
	{"Data Source host:"  $SourceHost}
	{"Readout program :"  $ReadoutPath}
    }

    pack .settings.top
    set label 0
    foreach item $labellist {
	set heading [lindex $item 0]
	set var     [lindex $item 1]
	eval append heading $var
	label .settings.top.l$label -text $heading 
	pack  .settings.top.l$label -anchor w
	incr label
    }
 
    button .settings.bottom.ok -text Accept -command "
    bind .settings <Destroy> {}
	destroy .settings

	Stager::SetHost  $tapehost
	Stager::SetDrive $TapeDrive
	set TapeCapacity $TapeCapacity	
	Experiment::SetFtpLogInfo $ftphost $Password

	ReadoutControl::SetReadoutProgram $SourceHost $ReadoutPath
	StartReadout $SourceHost $ReadoutPath $ftphost $Password

	StartStager $StageStrategy
        SetThreshold $threshold

	MainGui
    "
    button .settings.bottom.no -text Reject -command {
	bind .settings <Destroy> {}
	destroy .settings
	GuiGetSettings
    }
    pack .settings.bottom
    pack .settings.bottom.ok .settings.bottom.no -side left

   
}
#  Ensure we are a unique instance.  If not then we emit an error dialog
#  and exit Multiple instances of stager for the same user can cause data
#  loss!!!
#
if {![UniqueInstance::Unique Stager]} {
    toplevel .error
    set other [UniqueInstance::WhoElse Stager]
    append message "Another instance of stager pid@host = $other "
    append message "appears to be running.  "
    append message "If you are sure this is not the case, delete the file "
    append message "~/.Stager.lock and run this program again."
    tk_dialog .error "Duplicate Stager" \
	    $message error 0 "Dismiss"
    exit
}
#
#  Start out with toplevel hidden.
wm withdraw .


bind . <Destroy> "UniqueInstance::Exit Stager Main"

#  Try to recover settings from the configuration file
#
set file [ExpFileSystem::WhereisMetadata]
append file "/" $configfilename


if {[file exists $file]} {
    set thresh [ReadSettings $file]
    GuiConfirmSettings $thresh

} else {
    GuiGetSettings
}
ChangeTape


Experiment::CleanOrphans






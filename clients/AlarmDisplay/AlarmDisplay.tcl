#!/bin/sh
# Start Wish. \
exec tclsh ${0} ${@}


#==============================================================================
# Author:
#    Jason Venema
#    NSCL
#    Michigan State University
#    East Lansing, MI 48824-1321
#    mailto: venemaja@msu.edu
#
# This is the script for a tcl/tk sortable tablelist used as an alarm logger
# and displayer. Alarms are displayed in the tablelist and each alarm consists
# of a facility (the logging entity), a status (New, Acknowledged and
# Dismissed), a message (why there is an alarm), and a date/time at which the
# alarm was logged. Alarms are also written to the alarmn database file. The
# database is implemented using gdbm as described in the file CAlarmServer.h
# This script cannot access the database, but must make all transactions with
# the alarm server.  The server is the only entity that can read from or write
# to the database files.  This solves issues of concurrent reads/writes, as 
# several alarm displays may be running concurrently.
#==============================================================================

#  Ensure our library directories are part of the search path.
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

# Images are in the Images directory below the top.

set imagedir [file join $libdir Images]

puts "path: $auto_path"


package require Tk
package require Tablelist

puts [package version Tablelist]

namespace eval Alarm {}

#------------------------------------------------------------------------------
# proc Alarm::Display
#
# This procedure is responsible for creating the actual tablelist display
# widget that displays the alarms. All of the table options are set in this
# function.
#------------------------------------------------------------------------------
proc Alarm::Display {} {
    global tbl
    global expId
    global script_dir
    #
    # Create a top-level widget
    #
    set top .__configTop
    for {set n 2} {[winfo exists $top]} {incr n} {
        set top .__configTop$n
    }
    toplevel $top
    set title "NSCL Alarm Logger for Experiment $expId"
    set title [concat $title "on [exec hostname]"]
    wm title $top $title
    wm geometry $top 23x10+350+200

    #
    # Create a scrolled tablelist widget with 3 dynamic-width
    # columns, 1 static width column, and interactive sort capability
    # within the top-level. The message column is static width because
    # messages can be arbitrarily long. If a message doesn't fit in the
    # defined width, double click it.
    #
    set tbl $top.tbl
    set vsb $top.vsb
    set hsb $top.hsb
    tablelist::tablelist $tbl \
	-columns {0 "Alarm ID"
	    0  "Facility"
	    0  "Status"
	    35 "Message"
	    0  "Time Created"
	    0  "Time Changed"
	    0  "From"} \
        -labelcommand tablelist::sortByColumn \
        -xscrollcommand [list $hsb set] -yscrollcommand [list $vsb set] \
        -background white -selectbackground navy -selectforeground white \
        -height 15 -width 95 -setgrid yes -stretch all -selectmode extended
    foreach col {1 3 5} {
        $tbl columnconfigure $col -background beige
    }
    $tbl columnconfigure 0 -hide true
    scrollbar $vsb -orient vertical   -command [list $tbl yview]
    scrollbar $hsb -orient horizontal -command [list $tbl xview]
    
    #
    # Create a frame for some buttons
    #
    set f $top.f
    frame $f
    pack $f -side top -fill x

    #
    # Create some images for use as cool buttons
    #
    image create photo AckButton -format GIF \
	-file [file join $script_dir $::imagedir/AckButton.gif]
    image create photo DisButton -format GIF \
	-file [file join $script_dir $::imagedir/DisButton.gif]
    image create photo RemButton -format GIF \
	-file [file join $script_dir $::imagedir/RemButton.gif]
    image create photo HistButton -format GIF \
	-file [file join $script_dir $::imagedir/HistButton.gif]
    image create photo ExitButton -format GIF \
	-file [file join $script_dir $::imagedir/ExitButton.gif]

    #
    # Create a button for acknowledging 'new' alarms
    #
    set b1 $f.b1
    button $b1 -image AckButton -command [list Alarm::Acknowledge]
    pack $b1 -side left -expand yes

    #
    # Create a button for dismissing entries in the tablelist
    #
    set b2 $f.b2
    button $b2 -image DisButton -command [list Alarm::Dismiss]
    pack $b2 -side left -expand yes

    #
    # Create a button for removing dismissed alarms
    #
    set b3 $f.b3
    button $b3 -image RemButton -command [list Alarm::Remove]
    pack $b3 -side left -expand yes

    #
    # Create a button for creating a flat file containing all database info
    #
    set b4 $f.b4
    button $b4 -image HistButton -command [list Alarm::History]
    pack $b4 -side left -expand yes

    #
    # Create a button for exiting the display
    #
    set b5 $f.b5
    button $b5 -image ExitButton -command exit
    pack $b5 -side left -expand yes

    #
    # Bind double clicking to display the attributes of the event
    #
    set body [$tbl bodypath]
    bind $body <Double-1> [list Alarm::Display_Attrib]
    bind all <Control-h> [list Alarm::History]
    bind all <Control-x> {exit}
    bind all <Control-a> [list $tbl selection set 0 end]
    bind $body <Button-4> [list $tbl yview scroll -1 units]
    bind $body <Button-5> [list $tbl yview scroll 1 units]
    
    #
    # Manage the children of the top-level widget
    #
    grid $tbl -row 0 -column 0 -sticky news
    grid $vsb -row 0 -column 1 -sticky ns
    grid $hsb -row 1 -column 0 -sticky ew
    grid $f   -row 2 -column 0 -sticky ew -columnspan 2 -pady 10
    grid rowconfigure    $top 0 -weight 1
    grid columnconfigure $top 0 -weight 1
    return $top
}

#------------------------------------------------------------------------------
# proc Alarm::Display_Attrib
#
# This procedure will display the attributes of an alarm when the alarm is
# double clicked, by opening up a new window. The main purpose of this is to
# allow users to read messages that are too long to fit in the display.
#------------------------------------------------------------------------------
proc Alarm::Display_Attrib {} {
    global tbl
    global attrib_prompt
    global OkButton
    
    #
    # If the window already exists, create a new window for the newly
    # selected alarm.
    #
    if {[winfo exists .attrib]} {
	destroy .attrib
    }
    set row [$tbl get [$tbl curselection]]
    
    #
    # Create a new window for display
    #
    set f .attrib
    eval {toplevel $f} -width 80 -height 40
    wm title $f "Alarm Attributes"
    set top [frame $f.top -relief sunken -borderwidth 2]
    set bot [frame $f.bot]
    pack $top $bot -side top -fill both -expand true
    set t [text $top.t -setgrid true -wrap word \
	       -width 60 -height 15 -bg white \
	       -yscrollcommand [list $top.sy set]]
    scrollbar $top.sy -orient vert -command [list $top.t yview]
    pack $top.sy -side right -fill y
    pack $top.t -side left -fill both -expand true
    $t tag configure hang -lmargin1 0.0i -lmargin2 0.28i
    $t tag configure para -spacing1 0.1i
    $t insert end "Facility:[lindex $row 1]\n"
    $t insert end "State:    [lindex $row 2]\n"
    $t insert end "Message:  [lindex $row 3]\n"
    $t insert end "Created:  [lindex $row 4]\n\n"
    $t insert end "Changed:  [lindex $row 5]\n\n"
    $t insert end "From:    [lindex $row 6]"
    $t tag add hang 3.0 3.end
    for {set i 2} {$i <= 4} {incr i} {
	$t tag add para $i.0 $i.end
    }
    button $bot.ok -image OkButton -command {set attrib_prompt(ok) 1}
    pack $bot.ok -anchor n
    bind $f <Return> {set attrib_prompt(ok) 1}
	
    set attrib_prompt(ok) 0

    #
    # Wait for the user to interact with the dialog
    #
    Dialog_Wait $f attrib_prompt(ok) $f
    set result $attrib_prompt(ok)
    destroy $f
    if {$result} {
	return 
    }
}

#------------------------------------------------------------------------------
# proc Alarm::History
#
# This procedure prompts the user for a file name to write the history of this
# experiment to. History files are flat files containing information on every
# alarm that has been logged since the experiment began. The prompt is invoked
# by calling procedure Alarm::History_Prompt. This procedure uses the return
# value of that procedure to create a file for writing the history to.
#------------------------------------------------------------------------------
proc Alarm::History {} {
    global Host
    global expId
    global Reason
    global .history_prompt

    # If the window already exists, don't create another one.
    if {[winfo exists .history_prompt]} {
	raise .history_prompt
	return
    }

    # Get the history file the user supplies
    set histfile [tk_getSaveFile]
    if {($histfile != 0) && ($histfile != "")} {

	# Now we get the history from the server (database)
	set sock [socket $Host(ip) $Host(port)]
	fconfigure $sock -buffering line
	set packet [concat $expId $Reason(HISTORY)]
	puts $sock $packet
	    
	fileevent $sock readable {set can_read 1}
	vwait can_read
	set history [read $sock]
	flush $sock
	close $sock

	# Next, we open the file to write to. Note that, if the file
	# exists when we go to write, it is truncated!
	set fd [open $histfile w]
	set index 0
	while {[string compare [lindex $history $index] "eof"] != 0} {
	    set line1 [concat [lindex $history [expr $index + 2]] \
			   [lindex $history [expr $index + 3]]]
	    if {[lindex $history [expr $index + 5]] == "n"} {
		set status "New"
	    } elseif {[lindex $history [expr $index + 5]] == "a"} {
		set status "Acknowledged"
	    } elseif {[lindex $history [expr $index + 5]] == "d"} {
		set status "Dismissed"
	    } else {
		set status "Removed"
	    }
	    set line2 [concat [lindex $history [expr $index + 4]] $status]
	    set line3 [concat [lindex $history [expr $index + 6]] \
			   [lindex $history [expr $index + 7]]]
	    set line4 [concat [lindex $history [expr $index + 8]] \
			   [lindex $history [expr $index + 9]]]
	    set line5 [concat [lindex $history [expr $index + 10]] \
			   [lindex $history [expr $index + 11]]]
	    set line6 [concat [lindex $history [expr $index + 12]] \
			   [lindex $history [expr $index + 13]]]
	    set line7 [concat [lindex $history [expr $index + 14]] \
			   [lindex $history [expr $index +15]]]
	    set line8 [concat [lindex $history [expr $index + 16]] \
			   [lindex $history [expr $index + 17]]]
	    set index [expr $index + 18]
	    puts $fd [format "{$line1}\n{$line2}\n{$line3}\n{$line4}\n{$line5}\n{$line6}\n{$line7}\n{$line8}\n"]
	}
	flush $fd
	close $fd
    }
}

#------------------------------------------------------------------------------
# proc Alarm::History_Prompt
#
# This procedure displays the prompt, asking a user to enter the name of a
# file to print the experiment history to. If left blank, and the user presses
# the "OK" button, an error is given. Otherwise, the name entered is returned
# to the caller. Cancel does what cancel normally does -- destroys the dialog.
#
# BUGBUGBUG: Dialogs must be closed in reverse order in which they're opened
# I'm not sure if this is a bug or not, but the reason for it is that each
# dialog calls "tkwait". An outer caller to tkwait wont return until an inner
# nested call has returned, so the first dialog opened wont respond to events
# (like button presses) until the second one has been closed.
#------------------------------------------------------------------------------
proc Alarm::History_Prompt {} {
    global history_prompt
    global .history_prompt
    global histfile
    global OkButton
    global CancelButton

    set f .history_prompt
    if [Dialog_Create $f "Experiment Alarm History" -width 200 -height 90 \
	    -relief groove -borderwidth 2] {
	wm geometry $f 300x100+520+300
	set t [frame $f.top -height 30 -width 200 \
		   -relief groove -borderwidth 2]
	set m [frame $f.mid -height 30 -width 200]
	set b [frame $f.bot -height 30 -width 200]
	pack propagate $t false
	pack propagate $m false
	pack propagate $b false
	pack $t $m $b -side top -fill x
	label $m.bmp -bitmap info -fg navy
	label $t.tex -text "Please enter an output file name:" \
	    -wraplength 200 -justify left
	entry $m.entry -bg white -relief sunken -width 150 \
	    -textvariable histfile
	button $b.but1 -image OkButton -command {set history_prompt(ok) 1}
	button $b.but2 -image CancelButton -command {set history_prompt(ok) 0}
	pack $t.tex -side left -expand true -fill both
	pack $m.bmp $m.entry -side left -expand true -fill both
	pack $b.but1 $b.but2 -side left -expand true -fill both
	bind $f <Return> {set history_prompt(ok) 1}
	bind $f <Escape> {set history_prompt(ok) 0}
    }
    set history_prompt(ok) 0
    tkwait variable history_prompt(ok)
    set result $history_prompt(ok)
    destroy $f
    if {$result} {
	return $histfile
    } else {
	return 0
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Acknowledge
#
# This is the procedure that notifies the server that a given alarm has been
# acknowledged. The server makes the necessary changes to the database entry.
# Note that the server will only be contacted if the current status of the
# alarm is "New".
#------------------------------------------------------------------------------
proc Alarm::Acknowledge {} {
    global Host
    global expId
    global Reason
    global tbl
    foreach selection [$tbl curselection] {
	set row [$tbl get $selection]
	set id [lindex $row 0]
	if {[lindex $row 2] == "New"} {
	    set sock [socket $Host(ip) $Host(port)]
	    fconfigure $sock -buffering line
	    set packet [concat $expId $Reason(ACK) $id]
	    puts $sock $packet
	    flush $sock
	    close $sock
	}
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Dismiss
#
# Similar to acknowledge. In fact, the only difference is that the server will
# only be contacted if the current alarm status is "Acknowledged".
#------------------------------------------------------------------------------
proc Alarm::Dismiss {} {
    global Host
    global expId
    global Reason
    global tbl
    foreach selection [$tbl curselection] {
	set row [$tbl get $selection]
	set id [lindex $row 0]
	if {[lindex $row 2] == "Acknowledged"} {
	    set sock [socket $Host(ip) $Host(port)]
	    fconfigure $sock -buffering line
	    set packet [concat $expId $Reason(DIS) $id]
	    puts $sock $packet
	    flush $sock
	    close $sock
	}
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Remove
#
# This procedure removes an alarm with status "Dismissed from the displayer
# altogether. Once removed, it will not be redisplayed, though its existence
# will be maintained in the gdbm database.
#------------------------------------------------------------------------------
proc Alarm::Remove {} {
    global Host
    global expId
    global Reason
    global tbl
    foreach selection [$tbl curselection] {
	set row [$tbl get $selection]
	set id [lindex $row 0]
	if {[lindex $row 2] == "Dismissed"} {
	    set sock [socket $Host(ip) $Host(port)]
	    fconfigure $sock -buffering line
	    set packet [concat $expId $Reason(REM) $id]
	    puts $sock $packet
	    flush $sock
	    close $sock
	}
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Update
#
# This procedure notifies the server that it needs to update its records. The
# server responds with a list containing all of the information for the given
# experiment. We must then parse the list, and make sure that any new alarms
# are added to the display, and any alarm status changes are reflected in the
# display. Once the update has completed, the procedure waits for a period of
# time and then calls itself with the same parameters with which it was called
# in the first place.
#------------------------------------------------------------------------------
proc Alarm::Update {displaywin ms} {
    global Host
    global expId
    global Reason
    global NewAlarmsExist
    global GotFocus
    global RingBell
    global tbl

    set sock [socket $Host(ip) $Host(port)]
    fconfigure $sock -buffering line
    set packet [concat $expId $Reason(UPDATE)]
    puts $sock $packet

    fileevent $sock readable {set can_read 1}
    vwait can_read

    set alarmInfo [read $sock]
    flush $sock
    close $sock

    #
    # First, check if any of the alarms are new
    #
    set fieldIndex 5
    set NewAlarmsExist 0
    foreach field $alarmInfo {
	set status [lindex $alarmInfo $fieldIndex]
	set fieldIndex [expr $fieldIndex + 18]
	if {$status == "n"} {
	    set NewAlarmsExist 1
	}
    }
    if {!($NewAlarmsExist)} {
	set GotFocus 0
    }

    #
    # Now, check all the alarms that are currently displayed...
    set lastIndex 0
    set lastVal [lindex $alarmInfo $lastIndex]
    while {[string compare $lastVal "ALARMID:"] == 0} {
	set alarmID [lindex $alarmInfo [expr $lastIndex + 1]]
	
	# This regular expression evaluation is done so that if by chance
	# the alarm ID passed from the server has extra junk characters
	# appended to it, only the digit will be read. This solves the problem
	# of alarms already in the display being displayed repeatedly.
	regexp {[0-9]*} $alarmID aID
	set aID
	set alarmID $aID
	set alarmSTAT [lindex $alarmInfo [expr $lastIndex + 5]]
	if {$alarmSTAT == "n"} {
	    set alarmSTAT "New"
	} elseif {$alarmSTAT == "a"} {
	    set alarmSTAT "Acknowledged"
	} elseif {$alarmSTAT == "d"} {
	    set alarmSTAT "Dismissed"
	} else {
	    set alarmSTAT "Removed"
	}

	binary scan $alarmID c* IDVal
	set IDVal
	set alarmID [Convert_To_Int $IDVal]

	#
	# and set them to their appropriate values as stored in the database
	#	
	set i 0
	set found 0
	foreach tableRow [$tbl get 0 end] {
	    set tableSTAT [lindex $tableRow 2]
	    set tableID [lindex $tableRow 0]
	    binary scan $tableID c* TIDVal
	    set TIDVal
	    set tableID [Convert_To_Int $TIDVal]
	    if {($tableID == $alarmID)} {
		set found 1
		if {[string compare $tableSTAT $alarmSTAT] != 0} {
		    set fac [lindex $tableRow 1]
		    set mess [lindex $tableRow 3]
		    set created [lindex $tableRow 4]
		    if {$alarmSTAT == "Acknowledged"} {
			set changed [lindex $alarmInfo [expr $lastIndex + 11]]
		    } elseif {$alarmSTAT == "Dismissed"} {
			set changed [lindex $alarmInfo [expr $lastIndex + 13]]
		    } else {
			set changed [lindex $alarmInfo [expr $lastIndex + 15]]
		    }
		    set from [lindex $alarmInfo [expr $lastIndex + 17]]
		    $tbl delete $i
		    set entry [list $tableID $fac \
				   $alarmSTAT $mess $created $changed $from]
		    if {$alarmSTAT != "Removed"} {
			Display_Event $entry $i $displaywin
		    }
		}
	    }
	    incr i
	}

	#
	# If an alarm is not found in the displayer, but is in the database
	# (and it is not a dismissed alarm) then display it.
	#
	if {!($found)} {
	    set alarmFAC [lindex $alarmInfo [expr $lastIndex + 3]]
	    set alarmMESS [lindex $alarmInfo [expr $lastIndex + 7]]
	    set alarmMESS [string range $alarmMESS 1 end]
	    set alarmCREATED [lindex $alarmInfo [expr $lastIndex +9]]
	    set alarmCREATED [string range $alarmCREATED 1 end]

	    if {$alarmSTAT == "Acknowledged"} {
		set alarmCHANGED [lindex $alarmInfo [expr $lastIndex + 11]]
	    } elseif {$alarmSTAT == "Dismissed"} {
		set alarmCHANGED [lindex $alarmInfo [expr $lastIndex + 13]]
	    } else {
		set alarmCHANGED [lindex $alarmInfo [expr $lastIndex + 15]]
	    }
	    set alarmCHANGED [string range $alarmCHANGED 0 end]
	    set alarmFROM [lindex $alarmInfo [expr $lastIndex + 17]]
	    set entry [list $alarmID $alarmFAC $alarmSTAT $alarmMESS \
			   $alarmCREATED $alarmCHANGED $alarmFROM]
	    if {$alarmSTAT != "Removed"} {
		Display_Event $entry -1 $displaywin
	    }
	}
	set lastIndex [expr $lastIndex + 18]
	set lastVal [lindex $alarmInfo $lastIndex]
    }

    # 
    # Now check whether or not we need to ring the bell. If there are new
    # alarms in the display, the bell needs to be ringing.
    #
    if {$NewAlarmsExist} {
	incr RingBell
	if {[focus] != $displaywin &&
	    !($GotFocus)} {
	    if {[wm state $displaywin] == "iconic"} {
		wm deiconify $displaywin
	    } else {
		raise $displaywin
	    }
	    set GotFocus 1
	}
	if {$RingBell == 3} {
	    bell
	    set RingBell 0
	}
    }
    
    #
    # Now re-schedule ourself
    #
    after $ms [list Alarm::Update $displaywin $ms]
}

#------------------------------------------------------------------------------
# proc Alarm::Display_Event
#
# This procedure receives an entry to display in the tablelist, and the index
# at which to place it. The reason for the index is that, if an alarm status
# changes, updated alarm information should be displayed in the same row as
# the old alarm information. Without specifying where that was, the new alarm
# information will always appear at the bottom of the displayer. That can be
# annoying if one is trying to watch a specific alarm and it keeps shooting to
# the bottom of the list!
#------------------------------------------------------------------------------
proc Alarm::Display_Event {entry index displaywin} {
    global tbl
    global GotFocus
    set stat [lindex $entry 2]
    if {$index == -1} {
	$tbl insert end $entry
	if {$stat == "New"} {
	    $tbl cellconfigure end,2 -foreground red
	} elseif {$stat == "Acknowledged"} {
	    $tbl cellconfigure end,2 -foreground \#fd0
	} elseif {$stat == "Dismissed"} {
	    $tbl cellconfigure end,2 -foreground \#0d0
	}
	tablelist::sortByColumn $tbl 4
	tablelist::sortByColumn $tbl 4
    } else {
	$tbl insert $index $entry
	if {$stat == "New"} {
	    $tbl cellconfigure $index,2 -foreground red
	} elseif {$stat == "Acknowledged"} {
	    $tbl cellconfigure $index,2 -foreground \#fd0
	} elseif {$stat == "Dismissed"} {
	    $tbl cellconfigure $index,2 -foreground \#0d0
	}
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Dialog_Create
#
# Creates a dialog box if it does not already exist. This is a trick which 
# prevents having to re-create the widget each time the dialog is popped up
# and speeds things up a little. Adapted from "Practical Programming in Tcl 
# and Tk" by Brent Welch
#------------------------------------------------------------------------------
proc Alarm::Dialog_Create {top title args} {
    global dialog
    if [winfo exists $top] {
	switch -- [wm state $top] {
	    normal {
		raise $top
	    }
	    withdrawn -
	    iconified {
		wm deiconify $top
		catch {wm geometry $top $dialog(geo,$top)}
	    }
	}
	return 0
    } else {
	eval {toplevel $top} $args
	wm title $top $title
	return 1
    }
}

#------------------------------------------------------------------------------
# Alarm::Dialog_Wait
#
# This procedure gives focus to the dialog box which expects a user response
# and wont allow the user to do anything else until they have responded (i.e. 
# do something in another frame).
#------------------------------------------------------------------------------
proc Alarm::Dialog_Wait {top varName {focus {}}} {
    upvar $varName var
    bind $top <Destroy> [list set $varName $var]

    if {[string length $focus] == 0} {
	set focus $top
    }
    set old [focus -displayof $top]
    focus $focus
    catch {tkwait visibility $top}
    catch {raise $top}
    
    tkwait variable $varName
    focus $old
}

#------------------------------------------------------------------------------
# Alarm::Dialog_Dismiss
#
# Dismisses the dialog box from the screen.
#------------------------------------------------------------------------------
proc Alarm::Dialog_Dismiss {top} {
    global dialog
    catch {
	set dialog(geo,$top) [wm geometry $top]
	wm withdraw $top
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Set_Exp_Id
#
# This procedure's purpose is to invoke the prompt for an experiment id. Once
# the prompt has returned, we also check whether it returned a valid experiment
# id or not.
#------------------------------------------------------------------------------
proc Alarm::Set_Exp_Id {} {
    global expId
    global alarmId
    global Host
    global Reason
    global tbl
    global sure

    set expId [Alarm::Experiment_Id_Prompt]
    if {$expId != ""} {
	#
	# Before displaying, we initialize the displayer by figuring out 
	# how many entries are already in the alarm log file.  This will
	# be our value for alarmId.
	#
	set sock [socket $Host(ip) $Host(port)]
	fconfigure $sock -buffering line
	set packet "$expId $Reason(INIT)"
	
	#
	# We send the experiment id, and ask for the number of 
	# alarms in our experiment
	#
	puts $sock $packet
	flush $sock
	fileevent $sock readable {set can_read 1}
	vwait can_read

	#
	# We get the number of alarms and turn it from an ASCII character 
	# into an integer
	#
	set Id [read $sock]
	regexp {[0-9]*} $Id aID
	set aID
	set alarmId $aID
	flush $sock
	close $sock
	unset sock
	#binary scan $Id c* alarm_val
	#set alarm_val
	#set alarmId [Convert_To_Int $alarm_val]
	
	if {$alarmId == 0} {
	    #
	    # Make sure that whatever groggy-eyed user entered this experiment
	    # id is SURE that this is their experiment.
	    #
	    Alarm::Make_Sure_Prompt $expId
	    if {!($sure)} {
		Set_Exp_Id
	    } else {
		set sock [socket $Host(ip) $Host(port)]
		fconfigure $sock -buffering line
		set packet "$expId $Reason(CREATE)"
		puts $sock $packet
		flush $sock
		close $sock
		unset sock
	    }
	}
    } else {
	Warning_Prompt
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Get_Experiment_Id {}
# 
# Prompts the user to enter the identification number of their experiment. Any
# invalid id's are handled by the caller.
#------------------------------------------------------------------------------
proc Alarm::Experiment_Id_Prompt {} {
    global id_prompt
    global expId
    global tbl
    global OkButton
    global CancelButton
    set f .id_prompt
    set id 0

    if [Dialog_Create $f "Experiment" -borderwidth 1 -width 200 -height 90 \
	   -relief groove -borderwidth 2] {
	wm geometry $f 300x100+520+300
	set t [frame $f.top -width 200 -height 30 \
		   -relief groove -borderwidth 2]
	set m [frame $f.mid -width 200 -height 30]
	set b [frame $f.bot -width 200 -height 30]
	pack $t $m $b -side top -fill x
	pack propagate $t false
	pack propagate $m false
	pack propagate $b false
	label $t.label -text "Enter the experiment identification number:" \
	    -wraplength 400 -justify left
	label $m.bmp -bitmap info -fg navy 
	entry $m.entry -width 150 -bg white -relief sunken -textvariable expId
	button $b.but1 -image OkButton -command {set id_prompt(ok) 1}
	pack $t.label -side left -fill both -expand true
	pack $m.bmp $m.entry -side left -expand true -fill both
	pack $b.but1 -side right
	bind $f <Return> {set id_prompt(ok) 1}
    }
    set id_prompt(ok) 0
    Dialog_Wait $f id_prompt(ok) .id_prompt.mid.entry
    Dialog_Dismiss $f
    if {$id_prompt(ok) && $expId != ""} {
	return $expId
    } else {
	return ""
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Make_Sure_Prompt
#
# This is to make sure, after the user has entered the experiment id, that
# they entered the right one. This will hopefully avoid users accidentally
# tampering with experiments that are not theirs, as well as needless creation
# of new experiment keys with no alarms in the database files.
#------------------------------------------------------------------------------
proc Alarm::Make_Sure_Prompt {id} {
    global sure_prompt
    global sure
    global OkButton
    global CancelButton

    #
    # Create a new window for display
    #
    set sure_prompt(ok) 0
    set f .sure
    eval {toplevel $f} -width 300 -height 100
    wm geometry $f 300x100+520+300
    wm title $f "Are you sure?"
    set top [frame $f.top -relief groove -borderwidth 2 -width 300 -height 70]
    set bot [frame $f.bot -width 300 -height 30]
    pack $top $bot -side top -fill both -expand true
    pack propagate $top false
    pack propagate $bot false
    set tl [frame $top.lft -width 30 -height 70]
    set tr [frame $top.rt  -width 270 -height 70]
    pack $tl -side left
    pack $tr -side right
    pack propagate $tl false
    pack propagate $tr false

    set text "Experiment $id has never had any alarms logged to it."
    set text [concat $text "Are you sure you want to create this experiment?"]
    label $tl.bmp -bitmap question -fg navy
    label $tr.tex -text  $text -wraplength 240
    pack $tr.tex -side left -fill both -expand true
    pack $tl.bmp -side left -fill both -expand true
    button $bot.ok -image OkButton \
	-command {set sure 1;set sure_prompt(ok) 1}
    button $bot.no -image CancelButton \
	-command {set sure 0;set sure_prompt(ok) 0}
    pack $bot.no $bot.ok -side right
    bind $f <Return> {set sure 1;set sure_prompt(ok) 1}
    bind $f <Escape> {set sure 0;set sure_prompt(ok) 0}

    #
    # Wait for something to happen. Note that the return values are
    # inconsequential. The value of the global variable "sure", which is
    # set the moment a button (or return) is pressed is all that affects
    # what happens next. There was a good reason for doing it this way:
    # it didn't work any other way.
    #
    Dialog_Wait $f sure_prompt(ok) $f
    destroy $f
    if {$sure_prompt(ok)} {
	return 1
    } else {
	return 0
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Warning_Prompt
#
# This procedure prompt the user that an invalid experiment id has been entered
# and that no data can be retrieved from the server until a valid id is 
# entered. The user can opt to enter a valid id, or exit the program.
#------------------------------------------------------------------------------
proc Alarm::Warning_Prompt {} {
    global warning_prompt
    global expId
    global script_dir
    set f .warning_prompt

    #
    # Create the button images
    #
    image create photo RetryButton -format GIF \
	-file [file join $script_dir $::imagedir/RetryButton.gif]
    image create photo ExitButton -format GIF \
	-file [file join $script_dir $::imagedir/ExitButton.gif]

    if [Dialog_Create $f "Warning" -width 320 -height 70 \
	   -relief groove -borderwidth 2] {
	wm geometry $f 320x70+520+300
	set t [frame $f.top -height 40 -width 320 \
		   -relief groove -borderwidth 2]
	set b [frame $f.bot -height 30 -width 320]
	pack propagate $t false
	pack propagate $b false
	pack $t -side top -fill x
	pack $b -side bottom -fill x
	label $t.bmp -bitmap warning -fg \#fd0
	label $t.tex -text "You must enter a valid experiment ID to proceed!" \
	    -wraplength 300 -justify left
	button $b.but1 -image RetryButton -command {set warning_prompt(ok) 1}
	button $b.but2 -image ExitButton -command {set warning_prompt(ok) 0}
	pack $t.bmp $t.tex -side left -expand true -fill both
	pack $b.but2 $b.but1 -side right
	bind $f <Return> {set warning_prompt(ok) 1}
	bind $f <Escape> {set warning_prompt(ok) 0}
    }
    set warning_prompt(ok) 0
    Dialog_Wait $f warning_prompt(ok) $f
    Dialog_Dismiss $f
    if {$warning_prompt(ok)} {
	Set_Exp_Id
    } else {
	exit
    }
}

#------------------------------------------------------------------------------
# proc Alarm::Convert_To_Int
#
# This procedure converts character strings received from socket connections
# into integer values that can be manipulated arithemetically. This is 
# necessary because everything in tcl is, by default, a string of characters.
#------------------------------------------------------------------------------
proc Alarm::Convert_To_Int {nums} {
    set new ""
    foreach digit $nums {
	set new [linsert $new 0 $digit]
    }
    set decimal 1
    set alarmId 0
    foreach num $new {
	set dig [expr ($num - 48) * $decimal]
	set alarmId [expr $alarmId + $dig]
	set decimal [expr $decimal * 10]
    }
    return $alarmId
}

#------------------------------------------------------------------------------
# This is the end of the procedure definitions
#------------------------------------------------------------------------------

#
# These are the host and port numbers of the server
#
set Host(ip) [list localhost]

#
# NewAlarmsExist indicates whether there are new alarms in the display, in
# which case a bell must be sounded.
#
set NewAlarmsExist 0

#
# Enumeration of possible requests to the server
#
set Reason(LOG) 0
set Reason(ACK) 1
set Reason(DIS) 2
set Reason(REM) 3
set Reason(UPDATE) 4
set Reason(INIT) 5
set Reason(CREATE) 6
set Reason(HISTORY) 7

#
# See if the user wants to change the default alarm server host or port
#
set argument1 [lindex $argv 1]
set argument2 [lindex $argv 2]
set argument3 [lindex $argv 3]
set script_dir $argument3
if {[llength $argument2] == 0} {
    set script_dir $argument1
} elseif {[llength $argument3] == 0} {
    set script_dir $argument2
}

#
# Create some images for cool buttons
#
image create photo OkButton -format GIF \
    -file [file join $script_dir $::imagedir/OkButton.gif]
image create photo CancelButton -format GIF \
    -file [file join $script_dir $::imagedir/CancelButton.gif]

#
# Parse the command line arguments
#
if {[llength $argument1]} {
    if {[string range $argument1 0 1] == "-h" ||
	[string range $argument1 0 1] == "-p"} {
	if {[string range $argument1 0 1] == "-h"} {
	    set Host(ip) [string range $argument1 2 end]
	} elseif {[string range $argument1 0 1] == "-p"} {
	    set Host(port) [string range $argument1 2 end]
	}
    }
    if {[string range $argument2 0 1] == "-h" ||
	[string range $argument2 0 1] == "-p"} {
	if {[string range $argument2 0 1] == "-h"} {
	    set Host(ip) [string range $argument2 2 end]
	} elseif {[string range $argument2 0 1] == "-p"} {
	    set Host(port) [string range $argument2 2 end]
	}
    }
}
if [info exists Host(port)] {
    puts "Connecting to AlarmServerApp on host $Host(ip) on port $Host(port)"
    set Host(port) 2703
} else {
    set port_found 0
    if [catch {set file [open "/etc/services" r]} result] {
	puts -nonewline "No such file '/etc/services'. "
	put "Using default port 2703."
    } else {
	while {![eof $file]} {
	    gets $file line
	    if {[lindex $line 0] == "daqalarm"} {
		set proto [lindex $line 1]
		set index [string first "/" $proto 0]
		if {$index > 0} {
		    set Host(port) [string range $proto 0 [expr $index - 1]]
		    set port_found 1
		    break
		}
	    }
	}
    }
    if {$port_found == 1} {
	puts -nonewline "Found daqalarm port in '/etc/services'. "
	puts "Connecting to host $Host(ip) on port $Host(port)."
    } else {
	puts -nonewline "Port 'daqalarm' not configured in '/etc/services'. "
	puts "Using default port."
	set Host(port) 2703
	puts "Connecting to host $Host(ip) on port $Host(port)."
    }
}

#
# Now we build the displayer
#
Alarm::Set_Exp_Id
set displaywin [Alarm::Display]

#
# We repeatedly prompt the server to update our records
# Also, if a new alarm exists, then we need to raise the
# displayer to be the top window, or deiconify it if it has
# been iconified.
#
set GotFocus 1
set RingBell 2
Alarm::Update $displaywin 1000

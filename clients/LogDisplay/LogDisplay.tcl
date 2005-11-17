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


# (C) Copyright Michigan State University 1937, All rights reserved 
#==============================================================================
# Author:
#    Jason Venema
#    NSCL
#    Michigan State University
#    East Lansing, MI 48824-1321
#    mailto: venemaja@msu.edu
#
# This is the script for a tcl/tk sortable, filterable, recoverable tablelist
# which is used as an event logger and displayer. Events are displayed in the 
# tablelist and each event consists of a facility, a severity, a message and a
# date/time at which the event was logged. Events are also written to the log
# file. The logfile defaults to "daq.log" but can be changed with the command
# line option "-l<filename>. 
#==============================================================================

# Set up the auto path so we can find out packages...
# We assume that all packages are in the directory tree rooted in ..
# canonicalize .. and, if necessary, add it to auto_path.
#

namespace eval Logger {}

#------------------------------------------------------------------------------
# Logger::Log
#
# This procedure accesses the logfile and logs the entry. Events are
# logged via the following command: Logger::Log <Facility> <Severity> <Message>
# The severity can be one of three options: "Success", "Warning" or "Error".
# The return value is the time which the event was logged. This is useful for
# external programs which are connected to us via a socket and would like to
# display the event with the exact time at which it occurs.
#------------------------------------------------------------------------------
proc Logger::Log {entry} {
    global logfile

    # Break the entry down into its components
    set facility [lindex $entry 0]
    set severity [lindex $entry 1]
    set from     [lindex $entry 2]
    for {set i 3} {$i < [llength $entry]} {incr i} {
	lappend message [lindex $entry $i]
    }
    join $message
    set time [exec date "+%Y-%m-%d  %H:%M:%S %Z %a"]

    # Log the event to file
    if [catch {set file [open $logfile a]} result] {
	puts stderr $result
    } else {
	set line "FACILITY: $facility\nSEVERITY: $severity\nMESSAGE:  $message\nFROM:     $from\nTIME:     $time\n"
	puts $file $line
	close $file
    }
    return $time
}

#------------------------------------------------------------------------------
# Logger::Display
#
# This procedure actually creates the display and defines the buttons.
# Currently, supported actions are:
#    Filter... - Filters out messages by applying a pattern
#    Unfilter  - Redisplays events which have been filtered
#    Dismiss   - Removes the selected event from the display
#    Recover   - Redisplays all events which have been dismissed or which, for
#                any reason, failed to display when the event was logged. Reads
#                all events from the logfile and displays them.
#    Hosts...  - Edit the allowed host list
#    Exit      - Exit from the logger
#------------------------------------------------------------------------------
proc Logger::Display {} {
    global tbl
    global b2
    global b4
    global script_dir

    #
    # Create a top-level widget
    #
    set top .__configTop
    for {set n 2} {[winfo exists $top]} {incr n} {
	set top .__configTop$n
    }
    toplevel $top
    wm title $top "NSCL System Message Logger on [exec hostname]"

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
	-columns {0 "Facility"
	    0 "Severity"
	    35 "Message"
	    0 "Date/Time"
	    0 "From"} \
	-labelcommand tablelist::sortByColumn \
	-xscrollcommand [list $hsb set] -yscrollcommand [list $vsb set] \
	-background white -selectbackground navy -selectforeground white \
	-height 15 -width 95 -setgrid yes -stretch all -selectmode extended
    foreach col {1 3} {
	$tbl columnconfigure $col -background beige
    }
    scrollbar $vsb -orient vertical   -command [list $tbl yview]
    scrollbar $hsb -orient horizontal -command [list $tbl xview]

    #
    # Create a frame for some buttons
    #
    set f $top.f
    frame $f
    set ft [frame $f.top]
    set fb [frame $f.bot]
    pack $ft $fb -side top -fill x

    #
    # Create some images for use as cool buttons
    #
    image create photo FiltButton -format GIF \
	-file [file join $script_dir ../Images/FiltButton.gif]
    image create photo UfiltButton -format GIF \
	-file [file join $script_dir ../Images/UfiltButton.gif]
    image create photo DisButton -format GIF \
	-file [file join $script_dir ../Images/DisButton.gif]
    image create photo RecovButton -format GIF \
	-file [file join $script_dir ../Images/RecovButton.gif]
    image create photo HostButton -format GIF \
	-file [file join $script_dir ../Images/HostButton.gif]
    image create photo ExitButton -format GIF \
	-file [file join $script_dir ../Images/ExitButton.gif]

    #
    # Create a button for filtering
    #
    set b1 $ft.b1
    button $b1 -image FiltButton -command [list Logger::Filter]
    pack $b1 -side left -expand yes

    #
    # Create a button for unfiltering the entries in the tablelist
    #
    set b2 $ft.b2
    button $b2 -image UfiltButton -command [list Logger::Unfilter]
    pack $b2 -side left -expand yes
    $b2 configure -state disabled

    #
    # Create a button for dismissing entries in the tablelist
    #
    set b3 $ft.b3
    button $b3 -image DisButton -command [list Logger::Dismiss]
    pack $b3 -side left -expand yes

    #
    # Create a button for recovering a display from a logfile
    #
    set b4 $fb.b4
    button $b4 -image RecovButton -command [list Logger::Recover]
    pack $b4 -side left -expand yes
    $b4 configure -state disabled

    #
    # Create a button for editing the host list
    #
    set b5 $fb.b5
    button $b5 -image HostButton -command [list Logger::Read_Hostlist]
    pack $b5 -side left -expand yes

    #
    # Create a button for exiting the logger
    #
    set b6 $fb.b6
    button $b6 -image ExitButton -command {exit}
    pack $b6 -side left -expand yes

    #
    # Bind double clicking to display the attributes of the event
    #
    set body [$tbl bodypath]
    bind $body <Double-1> [list Logger::Display_Attrib]
    bind $body <Button-4> [list $tbl yview scroll -1 units]
    bind $body <Button-5> [list $tbl yview scroll 1 units]
    bind all   <Control-a> [list $tbl selection set 0 end]

    #
    # Manage the children of the top-level widget
    #
    grid $tbl -row 0 -column 0 -sticky news
    grid $vsb -row 0 -column 1 -sticky ns
    grid $hsb -row 1 -column 0 -sticky ew
    grid $f   -row 2 -column 0 -sticky ew -columnspan 2 -pady 10
    grid rowconfigure    $top 0 -weight 1
    grid columnconfigure $top 0 -weight 1

    # 
    # Since this procedure is only called once, we recover everything from
    # the logfile.
    #
    Logger::Recover
}

#------------------------------------------------------------------------------
# proc Logger::Read_Hostlist
#
# The purpose of this procedure is to call the Hostlist package procedure that
# reads in the list of allowed hosts for this machine, and authorizes them on
# the tclserver. This procedure ensures that no other windows can be opened
# until this one has been responded to.
#------------------------------------------------------------------------------
proc Logger::Read_Hostlist {} {
    global tbl

    set old [focus -displayof $tbl]
    set x [Hostlist::Read_List]
    focus $x
    catch {tkwait visibility $x}
    catch {grab $x}

    tkwait window $x
    catch {grab release $x}
    focus $old
}

#------------------------------------------------------------------------------
# proc Logger::Dismiss
#
# Dismisses the current selection from the tablelist. Once an entry has been
# dismissed, the only way to get it back is with a recover. This procedure
# checks whether or not there is currently an entry/entries selected and 
# deletes it/them. If no entry is selected, it is a no-op.
#
# Note: The whole business with incrementing i and subtracting it from the
#       selection index each time is because the value of each entry's index
#       decreases by 1 each time an entry is deleted. This is true because the
#       list "cur" is sorted first.
#------------------------------------------------------------------------------
proc Logger::Dismiss {} {
    global tbl
    global b4

    set cur [$tbl curselection]
    if {$cur != ""} {
	$b4 configure -state active
	set i 0
	lsort -integer $cur
	foreach sel $cur {
	    set sel [expr $sel - $i]
	    $tbl delete $sel
	    incr i
	}
    }
}

#------------------------------------------------------------------------------
# proc Logger::Display_Attrib
#
# This function is invoked when mouse button 1 is double clicked on an entry
# from within the body of the tablelist. It's purpose is to view the entry's
# attributes in the case that the "Message" field is too long to fit in the
# table. By offering a separate window to display long messages, it makes the
# tablelist look neater. It may turn out that this is unnecessary, but it
# couldn't hurt.
#------------------------------------------------------------------------------
proc Logger::Display_Attrib {} {
    global tbl
    global attrib_prompt
    global OkButton
    set row [$tbl get [$tbl curselection]]

    #
    # Create a new window for display
    #
    set f .attrib
    eval {toplevel $f} -width 80 -height 40
    wm title $f "Attributes of Logged Event"
    set top [frame $f.top -relief sunken -borderwidth 2]
    set bot [frame $f.bot]
    pack $top $bot -side top -fill both -expand true
    set t [text $top.t -setgrid true -wrap word \
	       -width 60 -height 15 -bg white \
	       -yscrollcommand [list $top.sy set]]
    scrollbar $top.sy -orient vert -command [list $top.t yview]
    pack $top.sy -side right -fill y
    pack $top.t -side left -fill both -expand true
    $t tag configure hang -lmargin1 0.0i -lmargin2 0.35i
    $t tag configure para -spacing1 0.1i
    $t insert end "Facility: [lindex $row 0]\n"
    $t insert end "Severity: [lindex $row 1]\n"
    $t insert end "Message:  [lindex $row 2]\n"
    $t insert end "Date:     [lindex $row 3]\n\n"
    $t insert end "From:     [lindex $row 4]"
    $t tag add hang 3.0 3.end
    for {set i 2} {$i <= 4} {incr i} {
	$t tag add para $i.0 $i.end
    }
    button $bot.ok -image OkButton -command {set attrib_prompt(ok) 1}
    pack $bot.ok -anchor n

    set attrib_prompt(ok) 0
    Dialog_Wait $f attrib_prompt(ok) $f
    destroy $f
    if {$attrib_prompt(ok)} {
	return 
    }
}

#------------------------------------------------------------------------------
# proc Logger::Filter
#
# This procedure is invoked from the displayer button labeled "Filter". It
# creates a top-level widget which prompts the user for information on what
# events they would like filtered. When return is pressed, procedure
# Perform_Filter is invoked to perform the actual dismissals (if any).
#------------------------------------------------------------------------------
proc Logger::Filter {} {
    global tbl
    global b2
    global hidden
    
    #
    # First we create a list of all facilities which have logged events
    # to pass to the filter dialog box. This allows the user to choose a
    # facility to filter from a list, rather than having to type the name in.
    # NOTE: This feature has not been implemented as of this version.
    #
    foreach row [$tbl get 0 end] {
	set present 0
	if {[info exists faclist]} {
	    foreach item $faclist {
		if {[lindex $row 0] == $item} {
		    set present 1
		}
	    }
	    if {!$present} {
		lappend faclist [lindex $row 0]
	    }
	} else {
	    lappend faclist [lindex $row 0]
	}
    }

    #
    # Create a Dialog prompt so we know what to filter
    #
    set options [Logger::Filter_Dialog_Prompt]
    if {$options == 0} {
	return
    }

    Logger::Perform_Filter $options
    if {[info exists hidden]} {
	$b2 configure -state active
    }
}

#------------------------------------------------------------------------------
# proc Logger::Perform_Filter
#
# This procedure performs the actual filtering. Some combination of list items
# (call options) is passed to us from the Filter_Dialog_Prompt which contains
# information on what item a user would like to have filtered. There are six
# possibilities:
#    - Show only this facility
#    - Hide only this facility
#    - Show only these severities
#    - Show only this facility of these severities
#    - Hide only this facility of these severities
#    - Show all entries...
# For each of these options, a date is also specified and only entries which
# occur either before or after that date will be displayed. For example, 
# option "Show all entries" doesn't do anything if no date is specified. On
# the other hand, option "Show only this facility..." shows only the facility
# requested whether a date is entered or not. The date is simply an additional
# constraint, but not a requirement.
#
# The entries which are "hidden" by the filter are stored in a global variable,
# name hidden, which will be used later to redisplay those item when an
# "unfilter" is requested.
#------------------------------------------------------------------------------
proc Logger::Perform_Filter {options} {
    global tbl
    global hidden
    global incoming
    
    #
    # We need to parse the options list
    #
    set facs    [lindex $options 0]
    set sevs    [lindex $options 1]
    set date    [lindex $options 2]
    set month   [lindex $date 0]
    set day     [lindex $date 1]
    set year    [lindex $date 2]

    # If the month or day begins with a "0", (e.g. 2002-02-02), then tcl
    # wont interpret it as a number, but as a string. So we have to
    # manually parse out the prefix '0'.
    #
    if {[string range $month 0 0] == "0"} {
	set month [string range $month 1 1]
    }
    if {[string range $day 0 0] == "0"} {
	set day [string range $day 1 1]
    }

    if {$month != ""} {
	set dateVal [expr $year*10000 + $month*100 + $day]
    } else {
	set dateVal 0
    } 
    set op1     [lindex $options 3]
    set op2     [lindex $options 4]

    #
    # If "incoming filter" was selected (checkbutton in the dialog box) then
    # we need to keep track of the options list so we can keep filtering later
    # when a new event is logged.
    #
    if {$incoming(on)} {
	set incoming(val) $options
    }

    #
    # Perform the actual filtering by hiding specified logged events
    #
    set i 0
    foreach row [$tbl get 0 end] {
	set tabledate [lindex $row 3]
	set month [string range $tabledate 5 6]
	set day   [string range $tabledate 8 9]
	set year  [string range $tabledate 0 3]
	if {[string range $month 0 0] == "0"} {
	    set month [string range $month 1 1]
	}
	if {[string range $day 0 0] == "0"} {
	    set day [string range $day 1 1]
	}
	set tabledateVal [expr $year*10000 + $month*100 +$day]

	#
	# Show only this facility...
	#
	if {$op1 == 0} {
	    if {$facs != [lindex $row 0] ||
		($facs == [lindex $row 0] &&
		 ($op2 == "before" && $dateVal <= $tabledateVal) ||
		 ($op2 == "after"  && $dateVal >= $tabledateVal))} {
		lappend hidden $row
		$tbl delete $i
	    } else {
		incr i
	    }
	    #
	    # Hide only this facility...
	    #
	} elseif {$op1 == 1} {
	    if {$facs == [lindex $row 0] &&
		(($op2 == "before" && $dateVal > $tabledateVal) ||
		 ($op2 == "after"  && $dateVal < $tabledateVal) ||
		 ($op2 == "none"))} {
		lappend hidden $row
		$tbl delete $i
	    } else {
		incr i
	    }
	    #
	    # Show only these severities...
	    #
	} elseif {$op1 == 2} {
	    if {(([lindex $sevs 0] != [lindex $row 1] &&
		  [lindex $sevs 1] != [lindex $row 1] &&
		  [lindex $sevs 2] != [lindex $row 1])) ||
		(([lindex $sevs 0] == [lindex $row 1] ||
		  [lindex $sevs 1] == [lindex $row 1] ||
		  [lindex $sevs 2] == [lindex $row 1]) &&
		 (($op2 == "before" && $dateVal <= $tabledateVal) ||
		  ($op2 == "after"  && $dateVal >= $tabledateVal)))} {
		lappend hidden $row
		$tbl delete $i
	    } else {
		incr i
	    }
	    #
	    # Show only this facility of these severities...
	    #
	} elseif {$op1 == 3} {
	    if {($facs != [lindex $row 0] ||
		 ([lindex $sevs 0] != [lindex $row 1] &&
		  [lindex $sevs 1] != [lindex $row 1] &&
		  [lindex $sevs 2] != [lindex $row 1]) ||
		 (($op2 == "before" && $dateVal <= $tabledateVal) ||
		  ($op2 == "after"  && $dateVal >= $tabledateVal))) ||
		($facs == [lindex $row 0] &&
		 ([lindex $sevs 0] == [lindex $row 1] ||
		  [lindex $sevs 1] == [lindex $row 1] ||
		  [lindex $sevs 2] == [lindex $row 1]) &&
		 (($op2 == "before" && $dateVal <= $tabledateVal) ||
		  ($op2 == "after"  && $dateVal >= $tabledateVal)))} {
		lappend hidden $row
		    $tbl delete $i
	    } else {
		incr i
	    }
	    #
	    # Hide only this facility of these severities...
	    #
	} elseif {$op1 == 4} {
	    if {($facs == [lindex $row 0] &&
		 ([lindex $sevs 0] == [lindex $row 1] ||
		  [lindex $sevs 1] == [lindex $row 1] ||
		  [lindex $sevs 2] == [lindex $row 1]) &&
		 (($op2 == "before" && $dateVal > $tabledateVal) ||
		  ($op2 == "after"  && $dateVal < $tabledateVal) ||
		  ($op2 == "none")))} {
		lappend hidden $row
		$tbl delete $i
	    } else {
		incr i
	    }
	    #
	    # Show all entries...
	    #
	} elseif {$op1 == 5} {
	    if {(($op2 == "before" && $dateVal <= $tabledateVal) ||
		 ($op2 == "after"  && $dateVal >= $tabledateVal))} {
		lappend hidden $row
		$tbl delete $i
	    } else {
		incr i
	    }
	}
    }
    return
}

#------------------------------------------------------------------------------
# proc Logger::Unfilter
#
# This procedure uses the global variable "hidden", which is a list of the rows
# which have been hidden from the displayer via filter operations, to restore
# the hidden entries. This is different from recover in that recover will
# perform a file read to restore all entries whether they've been filtered or
# dismissed, whereas unfilter will only restore entries which have been
# filtered but not dismissed.
#
# Note: Performing an unfilter causes incoming filter to be terminated as well
#------------------------------------------------------------------------------
proc Logger::Unfilter {} {
    global tbl
    global hidden
    global incoming
    global b2

    #
    # The first thing to do is to unset incoming filter (i.e. no more incoming
    # filtering will take place after an 'unfilter' command). Then we need to
    # see if anything has been filtered out by checking whether or not the
    # hidden array exists.
    #
    set incoming(on) 0
    if {[info exists hidden]} {
	for {set i 0} {$i < [llength $hidden]} {incr i} {
	    set row [lindex $hidden $i]
	    set entry [concat [lindex $row 0] [lindex $row 1] \
			   [lappend stuff [lindex $row 2] [lindex $row 3]] \
			   [lindex $row 4]]
	    Logger::Display_Event $entry
	    unset stuff
	}
	unset hidden
    }
    $b2 configure -state disabled
}

#------------------------------------------------------------------------------
# Logger::Recover
#
# This procedure allows a user to recover from a displayer restart. The logfile
# is accessed and its contents are read into local variables. The display's
# entries are searched and if the current logfile entry is not already in the
# tablelist, it is added to it. The same entry will thus never appear twice in
# the tablelist.
#------------------------------------------------------------------------------
proc Logger::Recover {} {
    global tbl
    global logfile
    global hidden
    global b4

    #
    # First access the logfile
    #
    if [catch {set file [open $logfile r]} result] {
	puts stderr $result
    } else {
	set done 0
	while {![eof $file] && !$done} {
	    for {set n 0} {$n < 5} {incr n} {
		gets $file line
		if {$line == ""} {
		    set done 1
		} else {
		    set item$n [string range $line 10 end]
		}
	    }
	    
	    set index 0
	    set present 0
	    if {!$done} {
		# 
		# Make sure the entry isn't already in the display!
		#
		foreach listEntry [$tbl get 0 end] {
		    if {$item0 == [lindex $listEntry 0] &&
			$item1 == [lindex $listEntry 1] &&
			$item2 == [lindex $listEntry 2] &&
			$item3 == [lindex $listEntry 4] &&
			$item4 == [lindex $listEntry 3]} {
			set present 1
		    }
		    incr index
		}
	    }
	    
	    # 
	    # If it isn't in the display yet, put it there
	    if {!$present && !$done} {
		Display_Event [lappend entry $item0 $item1 $item2 \
				   $item4 $item3]
		unset entry
	    }
	    gets $file junk
	}
    }
    #
    # This is necessary is case someone performs a recover, followed by an
    # unfilter. Once a recover has been performed, there's nothing that will
    # still be hidden by the filter
    #
    if {[info exists hidden]} {
	unset hidden
    }
    $b4 configure -state disabled
}

#------------------------------------------------------------------------------
# Logger::Display_Event
#
# This procedure insert the events into the tablelist for display. In the
# displayer, severity can have one of three different values: Success, Warning,
# or Error. Each severity is displayed in its own special color. If the array
# index incoming(on) is 1, then the user has previously indicated that they
# would like to incoming filter a set of conditions. Those conditions are
# stored in the list contained in incoming(val), and so after inserting the 
# entry into the displayer, a filter is immediately performed with those 
# criteria.
#------------------------------------------------------------------------------
proc Logger::Display_Event {entry} {
    global tbl
    global incoming

    #
    # Insert the entry into the tablelist displayer
    #
    $tbl insert end $entry
    set severity [lindex $entry 1]
    if {$severity == "Success"} {
	$tbl cellconfigure end,1 -foreground \#0d0
    } elseif {$severity == "Warning"} {
	$tbl cellconfigure end,1 -foreground \#fd0
    } else {
	$tbl cellconfigure end,1 -foreground red
    }

    #
    # If incoming filtering is selected, perform incoming filtering
    #
    if {$incoming(on)} {
	Logger::Perform_Filter $incoming(val)
    }
    $tbl sortbycolumn 3 -decreasing
}

#------------------------------------------------------------------------------
# proc Logger::Dialog_Create
#
# Creates a dialog box if it does not already exist. This is a trick which 
# prevents having to re-create the widget each time the dialog is popped up
# and speeds things up a little. Adapted from "Practical Programming in Tcl 
# and Tk" by Brent Welch
#------------------------------------------------------------------------------
proc Logger::Dialog_Create {top title args} {
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
# Logger::Dialog_Wait
#
# This procedure gives focus to the dialog box which expects a user response
# and wont allow the user to do anything else until they have responded (i.e. 
# do something in another frame).
#------------------------------------------------------------------------------
proc Logger::Dialog_Wait {top varName {focus {}}} {
    upvar $varName var
    bind $top <Destroy> [list set $varName $var]

    if {[string length $focus] == 0} {
	set focus $top
    }
    set old [focus -displayof $top]
    focus $focus
    catch {tkwait visibility $top}
    catch {grab $top}
    
    tkwait variable $varName
    catch {grab release $top}
    focus $old
}

#------------------------------------------------------------------------------
# Logger::Dialog_Dismiss
#
# Dismisses the dialog box from the screen.
#------------------------------------------------------------------------------
proc Logger::Dialog_Dismiss {top} {
    global dialog
    catch {
	set dialog(geo,$top) [wm geometry $top]
	wm withdraw $top
    }
}

#------------------------------------------------------------------------------
# Logger::Filter_Dialog_Prompt
#
# This procedure builds the actual filter dialog box and invokes Dialog_Wait
# to wait for a repsonse from the user. When Ok or Cancel is pressed, then
# control is returned to the caller and the filter options are returned to the
# calling function.
#------------------------------------------------------------------------------
proc Logger::Filter_Dialog_Prompt {} {
    global incoming
    global log_prompt
    global OkButton
    global CancelButton
    set f .log_prompt

    #
    # Check whether the dialog has already been created
    #
    if [Dialog_Create $f "Filter" -borderwidth 10 -height 180 -width 350] {

	#
	# These are the frames. By giving them short names, they are easier
	# to type, and keep track of. The letters stand for: 
	#   t - top
	#   b - bottom
	#   r - right
	#   l - left
	#   m - middle
	# So "t" is the top section of the toplevel frame, "tm" is the middle
	# section of frame "t", etc...
	#
	set t [frame $f.top -relief groove -borderwidth 4]
	set m [frame $f.mid -relief groove -borderwidth 4]
	set b [frame $f.bottom]

	set tt [frame $t.top -width 400 -height 35]
	set tm1 [frame $t.mid1 -width 400 -height 35]
	set tm2 [frame $t.mid2 -width 400 -height 35]
	set tb [frame $t.bot -width 400 -height 35]
	set mt [frame $m.top -width 400 -height 135 \
		   -relief ridge -borderwidth 2]
	set mb [frame $m.bot -width 400 -height 65 \
		   -relief ridge -borderwidth 2]

	set tm2t [frame $tm2.top]
	set tm2b [frame $tm2.bot]
	set mbt [frame $mb.top -width 400 -height 15]
	set mbb [frame $mb.bot -width 400 -height 70]
	set mbbl [frame $mbb.lft -width 50 -height 100]
	set mbbr [frame $mbb.rt  -width 350 -height 100]

	#
	# This allows us to set the height and width to exactly what we want
	#
	foreach frm "$tt $tm1 $tm2 $tb $mt $mb $mbt $mbb $mbbl $mbbr" {
	    pack propagate $frm false
	}

	pack $t $m $b -side top -fill x
	pack $tt $tm1 $tm2 $tb -side top -fill x
	pack $mt $mb -side top
	pack $tm2t $tm2b -side top -fill x
	pack $mbt $mbb -side top -fill x
	pack $mbbl $mbbr -side left -fill x

	#
	# Here are all the messages and buttons that appear in the dialog
	#
	message $tt.msg -text "Facility:   " -aspect 1000
	message $tm1.msg -text "Severity:" -aspect 1000
	message $tm2t.msg -text "Date:       " -aspect 1000
	message $tb.msg -text "Filter Incoming:" -aspect 1000
	message $mbt.msg -text "which were..." -aspect 1000
	entry $tt.entry1 -textvariable log_prompt(facility1) -bg white
	entry $tm2t.entry1 -textvariable log_prompt(month) -bg white -width 2
	entry $tm2t.entry2 -textvariable log_prompt(day) -bg white -width 2
	entry $tm2t.entry3 -textvariable log_prompt(year) -bg white -width 4
	label $tm2t.label1 -text "-"
	label $tm2t.label2 -text "-"
	checkbutton $tm1.b0 -text "Success" -fg \#0d0 \
	    -variable log_prompt(sevS) \
	    -onvalue Success -selectcolor blue -anchor sw
	checkbutton $tm1.b1 -text "Warning" -fg \#fd0 \
	    -variable log_prompt(sevW) \
	    -onvalue Warning -selectcolor blue -anchor sw
	checkbutton $tm1.b2 -text "Error" -fg red -variable log_prompt(sevE) \
	    -onvalue Error -selectcolor blue -anchor sw
	radiobutton $mt.rb0 -variable log_prompt(op1) \
	    -text "Show only this facility..." -value 0 \
	    -selectcolor blue
	radiobutton $mt.rb1 -variable log_prompt(op1) \
	    -text "Hide only this facility..." -value 1 \
	    -selectcolor blue
	radiobutton $mt.rb2 -variable log_prompt(op1) \
	    -text "Show only these severities..." \
	    -value 2 -selectcolor blue
	radiobutton $mt.rb3 -variable log_prompt(op1) \
	    -text "Show only this facility of these severities..." \
	    -value 3 -selectcolor blue
	radiobutton $mt.rb4 -variable log_prompt(op1) \
	    -text "Hide only this facility of these severities..." \
	    -value 4 -selectcolor blue
	radiobutton $mt.rb5 -variable log_prompt(op1) \
	    -text "Show all entries..." -value 5 -selectcolor blue
	radiobutton $mbbr.date1 -variable log_prompt(op2) \
	    -text "logged before this date." -value before \
	    -selectcolor blue
	radiobutton $mbbr.date2 -variable log_prompt(op2) \
	    -text "logged after this date." -value after -selectcolor blue
	label $tm2b.msg \
	    -text "                  MM - DD - YYYY" -wraplength 200
	checkbutton $tb.b0 -variable incoming(on) -onvalue 1 \
	    -offvalue 0 -selectcolor blue -anchor sw
	pack $tt.msg $tt.entry1 -side left -anchor nw
	pack $tm1.msg $tm1.b0 $tm1.b1 $tm1.b2 -side left -anchor nw
	pack $tm2t.msg $tm2t.entry1 $tm2t.label1 $tm2t.entry2 $tm2t.label2 \
	    $tm2t.entry3 -side left -anchor nw
	pack $tm2b.msg -side left -anchor n
	pack $tb.msg $tb.b0 -side left -anchor sw
	pack $mt.rb0 $mt.rb1 $mt.rb2 $mt.rb3 $mt.rb4 $mt.rb5 \
	    -side top -anchor nw
	pack $mbt.msg -side top -anchor nw
	pack $mbbr.date1 $mbbr.date2 -side top -anchor nw

	button $b.ok -image OkButton -command {set log_prompt(ok) 1}
	button $b.cancel -image CancelButton -command {set log_prompt(ok) 0}
	pack $b.cancel $b.ok -side right -anchor n
    }
    
    #
    # Now we have to wait for the user to do something 
    # (i.e. press "Ok" or "cancel")
    #
    set log_prompt(ok) 0
    set log_prompt(op1) -1
    set log_prompt(op2) none
    foreach item {log_prompt(sevS) log_prompt(sevW) log_prompt(sevE)} {
	set $item 0
    }
    Dialog_Wait $f log_prompt(ok) $f
    Dialog_Dismiss $f
    if {$log_prompt(ok)} {
	if {[string length $log_prompt(month)] == 0 ||
	    [string length $log_prompt(day)]   == 0 ||
	    [string length $log_prompt(year)]  == 0} {
	    set log_prompt(month) {} ; set log_prompt(day) {} ; \
		set log_prompt(year) {}
	}
	set severities [list $log_prompt(sevS) $log_prompt(sevW) \
			    $log_prompt(sevE)]
	set date [list $log_prompt(month) $log_prompt(day) $log_prompt(year)]
	set result [list $log_prompt(facility1) $severities $date \
			 $log_prompt(op1) $log_prompt(op2)]
	return $result
    } else {
	return 0
    }
}

#------------------------------------------------------------------------------
# This is the end of the function defs.
#------------------------------------------------------------------------------

#
# Check if the default "daq.log" file is there. If not, touch it so the user
# doesn't get an error message
#
set dir [exec ls]
foreach file $dir {
    if {$file == "daq.log"} {
	set logisthere 1
    }
}
if {![info exists logisthere]} {
    exec touch daq.log
}
set logfile [exec pwd]/daq.log
set incoming(on) 0
set argument1 [lindex $argv 1]
set argument2 [lindex $argv 2]
if {$argc == 2} {
    set argument ""
    set script_dir $argument1
} elseif {$argc == 3} {
    set argument $argument1
    set script_dir $argument2
}

#
# See if the user wants to access a different log file with the -l switch
#
if {[llength $argument]} {
    if {[string range $argument 0 1] == "-l"} {
	set logfile [string range $argument 2 end]
	puts "Using logfile $logfile"
	foreach file $dir {
	    if {$file == $logfile} {
		set newlogisthere 1
	    }
	}
	if {![info exists newlogisthere]} {
	    exec touch $logfile
	}
    } else {
	puts stderr "No such command line switch: [string range $argument 0 1]"
    }
}

#  Now that I know the script-dir I can set up the auto_path if needed
#  to have the libs... I assume that the tree rooted in $script_dir/..
#  can give me everything I need.  We do things this way
#  due to the funky way we have to be run with the tcl server
#  and all.
#

set libDir [file join $script_dir ..]

# Canonicalize the libDir:

set wd [pwd]
cd $libDir
set libDir [pwd];       #Lazy way to canonicalize.
cd $wd

if {[lsearch $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}

package require Tablelist
package require Hostlist


# 
# Create some button images for global use
#
image create photo OkButton -format GIF \
    -file [file join $script_dir ../Images/OkButton.gif]
image create photo CancelButton -format GIF \
    -file [file join $script_dir ../Images/CancelButton.gif]

Hostlist::Init_List
Logger::Display
wm withdraw .

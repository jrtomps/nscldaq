#!/bin/sh
#   NOTE:: tclserver must live in the path.
#   start tclserver on us. \
exec tclserver  -p2700  ${0} ${@}


#
#   Simple TCL scaler display:
#    Iteration 1:  Support creation of tabbed notebook containing scaler pages.
#                  Above the notebook is a title/status line.
#                  tcl 'commands':
#                       page name title
#
#   Iteration 2: Support mapping scaler names to parameters:
#                    scaler name id
#
#    Iteration 3:  Support single scaler displays:
#                tcl command:
#                   display_single page scaler.
#    Iteration 4:   Support display of ratios:
#                tcl command:
#                    display_ratio page numerator denominator
#    Iteration 5:   Display elapsed time as "d h:m:s"
#
#    Iteration 6:   Support end of run filing of data.
#
#
# Ensure that initially required variables exist:

set RunNumber       Unknown
set RunTitle        Unknown
set RunState        Unknown
set ElapsedRunTime  0
set ScalerDeltaTime 0
set Fakename     >>><<<
set HMStime        "0 00:00:00"
set ScalerLogDir  "."		;# Default is to log in current directory.

set InitialRunNumber unknown
set StartTime        unknown

# Establish my location so that I can source in additional scripts:


set me [info script]
set mydirectory [file dirname $me]

set IntervalCount 0

#  Source in the notebook and tabbed notebook sources:


source $mydirectory/notebook.tcl
source $mydirectory/tabnbook.tcl
source $mydirectory/mwutil.tcl
source $mydirectory/tablelist.tcl
source $mydirectory/tablelistSortByColumn.tcl
source $mydirectory/tablelistWidget.tcl

package require Tablelist

# Clear the statistics:
#
proc ClearStatistics {} {
	global SumSquares
	global IntervalCount
	
	catch {unset SumSquares}
    set IntervalCount 0
}

#  Update the running statistics
#  We maintain SumSquares, the sum of the
#  squares  of the rates.
#
proc UpdateStatistics {} {
	global SumSquares
	global IntervalCount
	global Scaler_Increments
	global ScalerDeltaTime

    if {$ScalerDeltaTime == 0} {
	return;				# End of run.
    }
	
    incr IntervalCount
	foreach element [array names Scaler_Increments] {
		set rate [expr 1.0*$Scaler_Increments($element)/$ScalerDeltaTime]
		set square [expr $rate*$rate]
		if {[array names SumSquares $element] == ""} {
			set SumSquares($element) $square;           # First time set.
		} else {
			set SumSquares($element) \
			    [expr $SumSquares($element) + $square]; #Accumulate
		}
	}
}
#
#   Compute the standard deviation from the average
#  for scaler i
#
proc StdDev {i} {
	global SumSquares
	global Scaler_Totals
	global ElapsedRunTime
	global IntervalCount

    if {($ElapsedRunTime == 0) || ($IntervalCount == 0)} {
	return 0
    }


	set mean [expr 1.0*$Scaler_Totals($i)/$ElapsedRunTime]
	set smean [expr $mean*$mean]
	set sqr  [expr $SumSquares($i)/$IntervalCount]

	
	return [expr sqrt(abs($sqr - $smean))]
}
# Procedures required by the client:


#
#   Updates the contents of the tables.  
#
proc Update {} {
    global Fakename
    global Pages
    global ElapsedRunTime
    global HMStime
   
    UpdateStatistics
    
    set sec [expr round($ElapsedRunTime)]
    set min [expr $sec/60]
    set hours [expr $min/60]
    set days  [expr $hours/24]
    set HMStime [format "%d %02d:%02d:%02d" \
	         $days  \
		 [expr $hours % 24] \
                 [expr $min % 60] \
		 [expr $sec % 60]]


    foreach page [array names Pages] {
	if {$page != $Fakename} {
	    UpdateTable $Pages($page).lines.table
	}
    }
    if {[info proc UserUpdate] != ""} {
	UserUpdate
    }
}
#
#   Called when the run begins.
#   We will reset the HMStime (hours/min/sec elapsed run time)
#   back to 0.
#   We store the run start time and the "initial run nubmer"
#   So that end run knows when the run started and that we
#   did see the begin run buffer for this run.
#
proc BeginRun {} {
    global HMStime
    global InitialRunNumber
    global StartTime
    global RunNumber

	ClearStatistics
    
    set HMStime "0 00:00:00"
    
    set InitialRunNumber $RunNumber
    set StartTime [clock format [clock seconds]]

    if {[info proc UserBeginRun] != "" } {
	UserBeginRun
    }
}
#
#  Called when the run ends.
#  We will log the current set of scaler totals to a file.
#  The file will be  named:
#    $ScalerLogDir/run$runnumber.scalers
#
#  It will have the form:
#               $RunTitle
#               Run: $RunNumber
#               Started: $StartTime
#               Ended:   <NOW>
#               Duration $HMStime
#    Scalername              Scaler Totals
#    -------------------------------------
#    name                     total
#    ...                      ...
#
#   Note that if InitialRunNumber != RunNumber
#   Start time is given as Unknown.
#
proc EndRun   {} {
    global RunNumber
    global RunTitle
    global ScalerLogDir
    global HMStime
    global Scaler_Totals
    global ScalerMap
    global Fakename
	global InitialRunNumber
	global StartTime
	global ElapsedRunTime
    #  Construct the log filename:


    set filename $ScalerLogDir/run$RunNumber.scalers
    set fd [open $filename w]

    puts $fd "                     $RunTitle"
    puts $fd "                     Run: $RunNumber"
    if {$RunNumber == $RunNumber} {
    	puts $fd "                     Started: $StartTime"
    } else {
    	puts $fd "                     Started: >Unknown<"
    }  
    puts $fd "                     Ended:  [clock format [clock seconds]]"
    puts $fd "                     Duration: $HMStime"
    puts $fd " Scaler Name        Scaler Total  Avg Rate     Std. Dev           "
    puts $fd "------------------------------------------------------------------"
    set fmt  " %11s  %11d  %11.2f  %11.2f"
    
    # Get the alphabetized list of scaler channels and put them out.
    set channels [lsort [array names ScalerMap]]
    foreach channel $channels {
	if {$channel != $Fakename} {
	    set   id $ScalerMap($channel)
	    if {$ElapsedRunTime != 0} {
		set   Average [expr $Scaler_Totals($id)/$ElapsedRunTime]
	    } else { 
		set Average 0
	    }
	    set   sigma [StdDev $id]
	    catch {set line [format $fmt $channel $Scaler_Totals($id) $Average $sigma]}
	    if {$line != ""} {puts $fd $line}
	}
    }
    close $fd

    if {[info proc UserEndRun] != ""} {
	UserEndRun
    }
}


proc PauseRun {} {}
proc ResumeRun {} {}
proc RunInProgress {} {}

#
#   The procedure below is used to format columns.
#   it is handed a format specifier and a list of up to 
#   two items to put in the column.  The format specifier
#   is used to format each item in the list.
#
proc FormatColumn {format list} {
    if {[llength $list] == 0} {
	return ""
    }
    set fmt "$format"
    if {[llength $list] == 2} {
	append fmt " $format"
	return [format $fmt [lindex $list 0] [lindex $list 1]]
    } else {
	return [format $fmt $list]
    }
}

#  Update a line with a single scaler..
#
proc UpdateSingle {widget line name} {
    global ScalerMap
    global Scaler_Totals
    global Scaler_Increments
    global ScalerDeltaTime

    if {[catch "set i $ScalerMap($name)"] == 0} {
	if {[catch "set totals $Scaler_Totals($i)"] == 0} {
	    set totals $Scaler_Totals($i)
	    set incr   $Scaler_Increments($i)
	    if {$ScalerDeltaTime != 0} {
		set rate   [expr $incr/$ScalerDeltaTime]
	    } else { 
		set rate "0"
	    }
	    set rate [expr int($rate)]
	    $widget cellconfigure $line,2 -text $rate
	    $widget cellconfigure $line,3 -text $totals
	}
    }    
}

proc UpdateRatio {widget line numerator denominator} {
    global ScalerMap
    global Scaler_Totals
    global Scaler_Increments
    global ScalerDeltaTime

    if {[catch "set n $ScalerMap($numerator)"] == 0} {
	if {[catch "set d $ScalerMap($denominator)"] == 0} {
	    set tn $Scaler_Totals($n)
	    set td $Scaler_Totals($d)

	    set in $Scaler_Increments($n)
	    set id $Scaler_Increments($d)
	    
	    if {$ScalerDeltaTime != 0} {
		set rn [expr $in/$ScalerDeltaTime]
		set rd [expr $id/$ScalerDeltaTime]
	    } else {
		set rn "0"
		set rd "0"
	    }
	    if {$td != 0} {
		set qt [expr ($tn*1.0)/$td]
	    } else {
		set qt "0"
	    }
	    if {($id != 0) && ($ScalerDeltaTime != 0)} {
		set qr [expr $rn*1.0/$rd]
	    } else {
		set qr "0"
	    }
	    set rn [expr int($rn)]
	    set rd [expr int($rd)]
	    $widget cellconfigure $line,2 -text  "$rn $rd"
	    set tn [expr int($tn)]
	    set tq [expr int($td)]
	    $widget cellconfigure $line,3 -text  "$tn $td"
	    $widget cellconfigure $line,4 -text "$qr $qt"
	}
    }
}

#  Update  line with a ratio of scalers.

#
#   Update a scaler table.
#   We rely on the fact that the first column contains a scaler name.
#
proc UpdateTable {widget} {
    global ScalerMap
    global Scaler_Totals
    global Scaler_Increments
    global ScalerDeltaTime

    set lines [$widget index end]
    #   Iterate through all lines in the table:
    for {set line 0} {$line < $lines} {incr line} {
	set numerator   [$widget cellcget $line,0 -text]
	set denominator [$widget cellcget $line,1 -text]
	if {$numerator != ""} {		# Numerator is blank on blank line.
	    if {$denominator == "" } {
		UpdateSingle $widget $line $numerator
	    } else {
		UpdateRatio $widget $line $numerator $denominator
	    }
	}
    }
}

#
#   The gui consists of:
#     A top frame containing title/status information.
#     A bottom frame containing the tabbed notebook.
#

proc SetupGui {top} {
    global RunNumber RunTitle RunState
    global HMStime ScalerDeltaTime
    set stat      [frame $top.status]
    set topstat   [frame $stat.top]
    set title     [frame $stat.title]
    
    set leftstat  [frame $topstat.left]
    set runnum    [frame $leftstat.runnum]
    set duration  [frame $leftstat.duration]
    
    set rightstat [frame $topstat.right]
    set state     [frame $rightstat.state]
    set interval  [frame $rightstat.interval]
	
    set book [frame $top.notebook]

    # Title frame contents:

    label $title.tl    -text "Title: "
    label $title.title -textvariable RunTitle

    label $runnum.rl    -text "Run Number: "
    label $runnum.run   -textvariable RunNumber

    label $state.sl    -text "Run state: "
    label $state.state -textvariable RunState

    label $duration.atl   -text "Length of run: "
    label $duration.atime -textvariable HMStime

    label $interval.dtl  -text "Scaler interval: "
    label $interval.dt   -textvariable ScalerDeltaTime

	# Set up the geometry of the top part of the display
 	#
 	#      Low level widgets...	
  
 	pack $title.tl     $title.title    -side left -anchor w -fill x -expand 1
 	pack $runnum.rl    $runnum.run     -side left -anchor w
 	pack $state.sl     $state.state    -side left -anchor w
 	pack $duration.atl $duration.atime -side left -anchor w
 	pack $interval.dtl $interval.dt    -side left -anchor w
 	
 	# Pack the subframes of the left frame:
 	
 	pack $runnum $duration -side top  -anchor w
 	pack $state  $interval -side top  -anchor w
 	
 	# Pack the left and right frames.
 	
 	pack $leftstat $rightstat -side left -fill both -expand 1 -anchor w

        # Pack the top and title frames:

        pack $topstat $title -side top -fill x -expand 1 -anchor w
 	
 	# finally pack the top and bottom frames, and the stat frame.
 	
 	pack $stat -side top


    #  Notebook frame contents:


    set notebook [tabnotebook_create $book.pages]
    pack $book.pages -side bottom -fill both -expand 1 
    pack $book       -side bottom -fill both -expand 1 

 
    return $notebook
    
}
#------------------ Configuration commands ---------------

#   page command - allows a user to add a page.  Pages are maintained in
#                  an array as follows:
#                  page(name) = widget_id
#                  The page is initially stocked with two frames:
#                    .title - contains the page title centered.
#                    .lines - contains any scaler lines added later.
#
#                    .lines are a table list megawidget.
#                        with vert and horiz. scroll bars attached.

set Pages($Fakename) ""               ;# not likely to be a page named this.
proc page {name title} {
    global Pages
    global Notebook

    # prevent duplicate pages:
   
    if {[array names Pages $name] == $name} {
	puts "Attempt to create duplicate page: $name ignored"
    } else {
	set newpage [tabnotebook_page $Notebook $name]
	set Pages($name) $newpage
	frame $newpage.title -relief groove
	set lines [frame $newpage.lines]

	# contents of the label frame.

	label $newpage.title.titleline -text $title

	# contents of the lines frame.

	set table $lines.table
	set vsb   $lines.vsb
	set hsb   $lines.hsb

	tablelist::tablelist $table \
		-columns {0 "Numerator" 
	                  0 "Denominator"
                          0 "Rate(s)"
	                  0 "Total(s)"
                          0 "Ratio \[rate total\]"} \
		-labelcommand tablelist::sortByColumn \
		-xscrollcommand [list $hsb set]       \
		-yscrollcommand [list $vsb set]       \
		-height 15 -width 95                  \
		-setgrid yes -stretch all \
		-font -b&h-lucidatypewriter-bold-r-normal-sans-*-100-*-*-m-*
	$table columnconfigure 2 -align right \
		-formatcommand {FormatColumn "% 7d"}
	$table columnconfigure 3 -align right \
		-formatcommand {FormatColumn "% 10d"}
	$table columnconfigure 4 -align right \
		-formatcommand {FormatColumn "% 10.3f"}
	scrollbar $vsb -orient vertical   -command [list $table yview]
	scrollbar $hsb -orient horizontal -command [list $table xview]

	# Lay this out in the notebook page:
	
	pack $newpage.title.titleline -anchor c -side top
	pack $newpage.title -side top -fill x


#	grid      $table -row 0 -column 0 -sticky nws
#	grid      $vsb -row 0 -column 1   -sticky nse
#	grid      $hsb -row 1 -column 0 -sticky ews
	pack  $vsb   -side right  -fill y 
	pack  $table -anchor n   -fill both -expand 1
	pack  $hsb   -side bottom -anchor s -fill x -expand 1
	pack $newpage.lines -side top -fill x -expand 1
	
    }
}

#
#    channel command - map a scaler name to a scaler id.  Scaler Ids are
#       just indices into the scaler arrays.
#
set ScalerMap($Fakename) ""          ;# Not likely to be a scaler named this..

proc channel {name id} {
    global ScalerMap
    if {[array names ScalerMap $name] == $name} {
	puts "Attempt to make a duplicate scaler map of $name to $id ignored"
    } else {
	set ScalerMap($name) $id
    }
}

#
#  Display a blank line on the page at the current position.
#
proc blank {page} {
    global Pages
    
    if {[array names Pages $page] != $page} {
	puts "blank: page $page does not exist... ignored"
	return
    }

    set page $Pages($page)
    set table $page.lines.table
    set entry ""
    $table insert end $entry
}

#
#   display_single command - display a single scaler's rates and totals as
#            the next item in a page.
#
#       display_single page scaler
#
proc display_single {page scaler} {
    global ScalerMap
    global Pages

    # Both the page and scaler must exist:

    if {[array names ScalerMap $scaler] != $scaler} {
	puts "display_single Scaler $scaler does not exist .. ignored"
	return
    }
    if {[array names Pages $page] != $page} {
	puts "display_single Page $page does not exist ... ignored"
	return
    }

    #  test stuff for now:

    set page $Pages($page)
    set table $page.lines.table
    set entry $scaler
    $table insert end $entry

}

#
#  display_ratio command  display a pair of scalers and their ratio on 
#      a single table line.
#      display_ratio page numerator denominator
#
proc display_ratio {page numerator denominator} {
    global ScalerMap
    global Pages
    
    # Validate page and both scalers:

    if {[array names Pages $page] != $page} {
	puts "display_ratio Page $page does not exist ... ignored."
	return
    }
    if {[array names ScalerMap $numerator] != $numerator} {
	puts "display_ratio Numerator $numerator does not exist... ignored"
	return
    }
    if {[array names ScalerMap $denominator] != $denominator} {
	puts "display_ratio Denominator $denominator does not exist... ignored"
	return
    }

    # Create the table entry and let the update figure out the scaler values:

    set page $Pages($page)
    set table $page.lines.table
    $table insert end "$numerator $denominator"
    
}


set Notebook [SetupGui ""]



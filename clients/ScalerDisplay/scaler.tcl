#!/bin/sh
#   NOTE:: tclserver must live in the path.
#   start tclserver on us. \
exec tclserver  -pManaged -a"Scaler Display"  ${0} ${@}


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
#    Iteration 7:   Add:
#                    - Alarming
#                    - Strip charts.
#                    - Final rate == average rate.
#
#
# Ensure that initially required variables exist:

package require BLT
namespace import ::blt::tabset
namespace import ::blt::vector
namespace import ::blt::stripchart

#   The run state variable depends on whether or not
#   we're running inside of SpecTcl or as a 
#   TclServer for ScalerClient:

if {[info exists SpecTclHome]} {
    set RunStateName ScalerRunState
} else {
    set RunStateName RunState
}

set RunNumber       Unknown
set RunTitle        Unknown
set $RunStateName   Unknown
set ElapsedRunTime  0
set ScalerDeltaTime 0
set Fakename     >>><<<
set HMStime        "0 00:00:00"
set ScalerLogDir  "."		;# Default is to log in current directory.

set InitialRunNumber unknown
set StartTime        unknown

set pageSerial        0         ;# serial number used to name page frames.


#  The following globals are used for alarming:
#    lowAlarms(channel-name)   If this exists it contains the
#                              low level for the alarm.
#    hiAlarms(channel-name)    If this exists, it containst the
#                              hi level for the alarm.
#    pageAlarmState(pageName)  Contains the alarm mask for a page.
#                              Bits are as follows:
#                               1  - There is a channel in low alarm
#                               2  - There is a channel in high alarm.
#    lowColor                  Color when low alarm condition is true.
#    hiColor                   Color when hi alarm condition is true
#    bothColor                 Color when both alarm conditions are true.
#    silentAlarms              If true, alarms are silent.
set lowAlarmBit   1
set hiAlarmBit    2
set lowColor   green
set hiColor    red
set bothColor  orange
set silentAlarms 0

#  The global variables below configure the strip chart
# recorder.
#    stripChartChannels  - list of channels to strip chart.
#    stripChartRatios    - list of numerator/denominator pairs for
#                          strip chart rate ratios. vector names are
#                          numerator_over_denominator
#    yAxisType           - "log" or "linear"
#    xAxisLength         - Seconds of elapsed time on Xaxis.
#    stripchartWidget    - Path to the strip chart if defined.
#    timeVector          - Vector containing elapsed time points.
#    stripColors         - Foreach line style, cycle amongst these
#                          colors.
#    stripStyles         - -dash descriptions for strip chart data element.
#    stripColorIndex     - Index into color array.
#    stripStyleIndex     - Index into dash style array.
#
#    timeVector          - Vector of elapsed run times at scaler readouts.
#    legendPosition      - Position of legend (top bottom, left or right).
set stripchartChannels ""
set stripchartRatios   ""
set yAxisType          linear
set xAxisLength        3600
set stripchartWidget   ""
set logButton          ""
set stripColors {black red green blue cyan magenta}
set stripStyles { "" {5 5} {2 2} {5 5 2 2} {5 2} {5 5 2 5}}
set stripColorIndex 0
set stripStyleIndex 0
set legendPosition top

vector create timeVector


# Establish my location so that I can source in additional scripts:


set me [info script]
set mydirectory [file dirname $me]

set IntervalCount 0

#  Source in the notebook and tabbed notebook sources:



source $mydirectory/mwutil.tcl
source $mydirectory/tablelist.tcl
source $mydirectory/tablelistSortByColumn.tcl
source $mydirectory/tablelistWidget.tcl

package require Tablelist

# Compute a scaler rate.
#
#Parameters:
#   name    - name of the channel to compute
#Implicit inputs:
#  Scaler_Increments
#  ScalerDeltaTime
#  ScalerMap
#Returns:
#   Rate or 0 if rate cannot be determined (e.g. ScalerDeltaTime = 0).
#
proc scalerRate name {
    global Scaler_Increments
    global ScalerDeltaTime
    global ScalerMap

    # Not in map? return 0.

    if {[array names ScalerMap $name] == ""} {
        return 0
    }
    set channel $ScalerMap($name)

    # No such increment? return 0:

    if {[array names Scaler_Increments $channel] == ""} {
        return 0
    }


    if {$ScalerDeltaTime == 0} {
        return 0
    } else {
        return [expr 1.0*$Scaler_Increments($channel)/$ScalerDeltaTime]
    }
}

#
#  Get the name of a ratio strip chart vector
#
proc ratioName {numerator denominator} {
    append name $numerator _over_ $denominator
    return $name
}

#   Choose a style given the current values of
#   stripColorIndex, stripStyleIndex and the stripColors and
#   stripStyles globals.  First we iterate through the colors and
#   then through the line styles.  If we ever exhaust themm, we
#   start all over again.
# Returns:
#   A 2 element list consisting of the color and dash style.
#
proc selectElementStyle {} {
    global stripColors
    global stripStyles
    global stripColorIndex
    global stripStyleIndex

    # Selec the color and linestyle.

    set color  [lindex $stripColors $stripColorIndex]
    set dashes [lindex $stripStyles $stripStyleIndex]

    set result [list $color $dashes]

    #  Advance to the next color/style pair.

    incr stripColorIndex
    if {$stripColorIndex >= [llength $stripColors]} {
        set stripColorIndex 0
        incr stripStyleIndex
        if {$stripStyleIndex >= [llength $stripStyles]} {
            set stripStyleIndex 0
        }
    }
    return $result
}

# Clear the strip chart if it's defined.  This just involves
# clearing the vectors.
#
proc clearStripChart {} {
    global timeVector
    global stripchartChannels
    global stripchartRatios

    timeVector length 0

    foreach vec $stripchartChannels {
        global $vec
        $vec length 0
    }
    foreach pair $stripchartRatios {
        set name [ratioName [lindex $pair 0] [lindex $pair 1]]
        global $name
        $name length 0
    }
}

# Flip the scale of the stripchart widget from
# Auto x scale to strip chart mode.
# Parameters:
#    widget - path to the strip chart widget.
# Implicit inputs:
#    xAxisLength - Seconds on X axis in strip chart mode.
#
proc flipXScale {widget} {
    global xAxisLength

    # The shiftby gives it away:

    set current [$widget axis cget x -shiftby]

    if {$current != 0} {
        # Go to auto from strip chart mode.

        $widget axis configure x -autorange 0 -shiftby 0

    } else {
        # Go to strip chart from auto mode.

        $widget axis configure x -autorange $xAxisLength \
            -shiftby [expr 0.1*$xAxisLength]
    }
}

#
#  Create the strip chart frame. The strip chart frame
#  contains a stripchart widget and a checkbutton that
#  flips the y axis between log and linear.
#Parameter:
#  top    - The parent widget of the strip chart frame.
#Returns:
#  The strip chart widget.
proc createStripChart {top} {
    global stripchartWidget
    global logButton

    # Top as the initial toplevel is special:

    if {$top == "."} {
        set top ""
    }

    set stripframe       [frame $top.stripframe]
    set controlframe     [frame $top.stripframe.controls]
    set stripchartWidget [stripchart $stripframe.chart -height 2i -width 8i]
    set logButton [checkbutton $controlframe.loglin -command "flipLogLin $stripchartWidget" \
                               -text "Log Scale"]
    set scaleButton [checkbutton $controlframe.autochart -command "flipXScale $stripchartWidget" \
                                -text "Auto X scale"]

    pack $scaleButton $logButton -side left
    pack $stripchartWidget $controlframe -side top
    pack $stripframe -side top

    return $stripchartWidget
}
# Configure the stripchart widget according to the
# current state of the strip chart control variables.
#
# Implicit inputs:
#    yAxisType           - "log" or "linear"
#    xAxisLength         - Seconds of elapsed time on Xaxis.
#    stripchartWidget    - Path to the strip chart if defined.
#
proc configureStripChart {} {
    global stripchartWidget
    global yAxisType
    global xAxisLength
    global logButton
    global legendPosition

    $stripchartWidget legend configure -position $legendPosition
    set shiftby [expr $xAxisLength * 0.1]
    $stripchartWidget axis configure x -autorange $xAxisLength \
                                       -shiftby $shiftby \
	                               -title seconds
    if {$yAxisType == "linear"} {
        set log 0
        $logButton deselect
    } else {
        set log 1
        $logButton select
    }
    $stripchartWidget axis configure y -logscale $log -title counts

    puts "done"
}

# Flip the stripchart widget's y axis between log/lin mode
#
# Parameters;
#   widget    - The strip chart widget path.
# Implicit outputs:
#   yAxisType  - new type of axis.
proc flipLogLin widget {
    global yAxisType

    set current [$widget axis cget y -logscale]
    if {$current} {
        $widget axis configure y -logscale 0 -min 0
        set yAxisType linear
    } else {
        $widget axis configure y -logscale 1 -min 0.1
        set yAxisType log
    }
}
# Update the strip chart. This involves adding a new point
# to the timeVector and updating the rates in each of the vectors
# named in stripchartChannel, and the ratios defined by
# stripchartRatios.
#
#
proc updateStripChart {} {
    global ElapsedRunTime
    global Scaler_Increments
    global ScalerDeltaTime
    global ScalerMap
    global stripchartChannels
    global stripchartRatios
    global timeVector

    set timeVector(++end) $ElapsedRunTime

    foreach channel $stripchartChannels {
        global $channel
        set rate [scalerRate $channel]
        set ${channel}(++end) $rate
    }

    foreach pair $stripchartRatios {
        set numerator    [lindex $pair 0]
        set denominator  [lindex $pair 1]

        set nrate [scalerRate $numerator]
        set drate [scalerRate $denominator]
        if {$drate == 0} {
            set rate 0.0
        } else {
            set rate [expr 1.0*$nrate/$drate]
        }
        set vecname [ratioName $numerator $denominator]
        global $vecname
        set ${vecname}(++end) $rate
    }
}


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
#   Does most of the update logic
#
# Parameters:
#    average - true implies that the update will use run
#              averages for rates rather than interval rates.
#
proc DoUpdate {{average 0}} {
    global Fakename
    global Pages
    global ElapsedRunTime
    global HMStime

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
	    UpdateTable $Pages($page).lines.table $page $average
	}
    }
    bellOnAlarm
}

#
#   Updates the contents of the tables.
#

proc Update {} {
    global RunStateName
    global $RunStateName
    global stripchartWidget

    set State [set $RunStateName]

    UpdateStatistics
    if {$stripchartWidget != ""} {
        updateStripChart
    }

    if {$State == "Active" } {
        DoUpdate
    } else {
        DoUpdate 1
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
    global stripchartWidget

    ClearStatistics

    set HMStime "0 00:00:00"

    set InitialRunNumber $RunNumber
    set StartTime [clock format [clock seconds]]

    if {$stripchartWidget != ""} {
        clearStripChart
    }

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

#  Ring the bell if there are any alarms set.
proc bellOnAlarm {} {
    global pageAlarmState
    global silentAlarms

    if {$silentAlarms} {
        return
    }

    foreach page [array names pageAlarmState] {
        if {$pageAlarmState($page) != 0} {
	    bell
            return
        }
    }
}

#   Check the alarm state of a chanel.
# Parameters:
#   name  - Name of the channel.
#   value - Channel rate value.
# Returns:
#   0           - Channel not in alarm state.
#   lowAlarmBit - Channel in low rate alarm.
#   hiAlarmBit  - Channel in high rate alarm.
#
proc checkAlarms {name value} {
    global lowAlarmBit
    global hiAlarmBit
    global lowAlarms
    global hiAlarms

    set result 0

    if {[array names lowAlarms $name] != ""} {
        if {$value < $lowAlarms($name)} {
            set result $lowAlarmBit
        }
    }
    if {[array names hiAlarms $name] != ""} {
        if {$value > $hiAlarms($name)} {
            set result $hiAlarmBit
        }
    }
    return $result
}

#
#  Return the correct color given an alarm mask, and
#  a widget that can supply the 'normal' background color.
#
# Parameters:
#   mask       - The alarm mask that can contain an or of lowAlarmBit or hiAlarmBit.
# Returns:
#   The correct cell color.
#
proc alarmColor {mask} {
    global lowColor
    global hiColor
    global bothColor
    global lowAlarmBit
    global hiAlarmBit
    global Notebook

    # Mask out all bits other than the alarm bits:

    set mask [expr $mask & ($lowAlarmBit | $hiAlarmBit)]

    # The normal color will be the background color of
    # the notebook widget.

    set normalColor [$Notebook cget -background]

    if {$mask == 0} {
        return $normalColor
    }
    if {$mask == $lowAlarmBit} {
        return $lowColor
    }
    if {$mask == $hiAlarmBit} {
        return $hiColor
    }
    return $bothColor
}

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
#  If average true, then rates become average rates (end or run e.g.).
#Parameters:
#   widget       - table widget we're working on.
#   line         - Line within the table.
#   page         - Name of the page we're working on.
#   average      - nonzero to use average rates rather than
#                  interval rates.
proc UpdateSingle {widget line name page {average 0}} {
    global ScalerMap
    global Scaler_Totals
    global Scaler_Increments
    global ScalerDeltaTime
    global ElapsedRunTime
    global pageAlarmState

    if {[catch "set i $ScalerMap($name)"] == 0} {
	if {[catch "set totals $Scaler_Totals($i)"] == 0} {
	    set totals $Scaler_Totals($i)
	    set incr   $Scaler_Increments($i)
            set delta  $ScalerDeltaTime
            if {$average} {
                set incr $Scaler_Totals($i)
                set delta $ElapsedRunTime
            }
	    if {$delta != 0} {
		set rate   [expr 1.0*$incr/$delta]
	    } else {
		set rate "0"
	    }
	    $widget cellconfigure $line,2 -text $rate
	    $widget cellconfigure $line,3 -text $totals

            # Check the alarms:

            set alarms [checkAlarms $name $rate]
            set pageAlarmState($page) [expr $pageAlarmState($page) | $alarms]
            $widget cellconfigure $line,2 -background [alarmColor $alarms]
            $widget cellconfigure $line,0 -background [alarmColor $alarms]
	}
    }

}


#  Update a ratio line on the display.  If average is true,
#  rates become average rates (end of run e.g.).
# Parameters:
#    widget:      Table widget id.
#    line         Number of the line to work on.
#    numerator    Name of the numerator scaler.
#    denominator  Name of the denominator scaler.
#    page         Name of the page we're in.
#    average      If true, use average rates, not interval rates.
#
proc UpdateRatio {widget line numerator denominator page {average 0}} {
    global ScalerMap
    global Scaler_Totals
    global Scaler_Increments
    global ScalerDeltaTime
    global ElapsedRunTime
    global pageAlarmState

    if {[catch "set n $ScalerMap($numerator)"] == 0} {
	if {[catch "set d $ScalerMap($denominator)"] == 0} {
	    set tn $Scaler_Totals($n)
	    set td $Scaler_Totals($d)

	    set in $Scaler_Increments($n)
	    set id $Scaler_Increments($d)
            set delta $ScalerDeltaTime
            if {$average} {
                set in $Scaler_Totals($n)
                set id $Scaler_Totals($d)
                set delta $ElapsedRunTime
            }

	    if {$delta != 0} {
		set rn [expr 1.0*$in/$delta]
		set rd [expr 1.0*$id/$delta]
	    } else {
		set rn "0"
		set rd "0"
	    }
	    if {$td != 0} {
		set qt [expr ($tn*1.0)/$td]
	    } else {
		set qt "0"
	    }
	    if {($id != 0) && ($delta != 0)} {
		set qr [expr $rn*1.0/$rd]
	    } else {
		set qr "0"
	    }
	    $widget cellconfigure $line,2 -text  "$rn $rd"
	    $widget cellconfigure $line,3 -text  "$tn $td"
	    $widget cellconfigure $line,4 -text "$qr $qt"

            # Check the alarms:

            set alarm1 [checkAlarms $numerator $rn]
            set alarm2 [checkAlarms $denominator $rd]
            set alarms [expr $alarm1 | $alarm2]

            set pageAlarmState($page) [expr $pageAlarmState($page) | $alarms]
            $widget cellconfigure $line,2 -background \
                    [alarmColor $alarms]
            $widget cellconfigure $line,0 -background [alarmColor $alarm1]
            $widget cellconfigure $line,1 -background [alarmColor $alarm2]

	}
    }
}



#
#   Update a scaler table.
#   We rely on the fact that the first column contains a scaler name.
# Parameters:
#    widget    - The table wiget to update.
#    page      - The short name of the page being updated.
#    average != 0 implies that run averaged rates be used.
#
proc UpdateTable {widget page {average 0}} {
    global ScalerMap
    global Scaler_Totals
    global Scaler_Increments
    global ScalerDeltaTime
    global pageAlarmState
    global Notebook

    set pageAlarmState($page) 0

    set lines [$widget index end]
    #   Iterate through all lines in the table:
    for {set line 0} {$line < $lines} {incr line} {
	set numerator   [$widget cellcget $line,0 -text]
	set denominator [$widget cellcget $line,1 -text]
	if {$numerator != ""} {		# Numerator is blank on blank line.
	    if {$denominator == "" } {
		UpdateSingle $widget $line $numerator $page $average
	    } else {
		UpdateRatio $widget $line $numerator $denominator $page $average
	    }
	}
    }
    $Notebook tab configure $page -background \
            [alarmColor $pageAlarmState($page)]
}

#
#   The gui consists of:
#     A top frame containing title/status information.
#     A bottom frame containing the tabbed notebook.
#

proc SetupGui {top} {
    global RunNumber RunTitle 
    global RunStateName
    global $RunStateName
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
    label $state.state -textvariable $RunStateName

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


    set notebook [tabset $book.pages -tiers 3 -tearoff 0]
    pack $book.pages -side top -fill both -expand 1
    pack $book       -side top -fill both -expand 1


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
    global pageSerial

    # prevent duplicate pages:

    if {[array names Pages $name] == $name} {
	puts "Attempt to create duplicate page: $name ignored"
    } else {
        set newpage [frame $Notebook.p${pageSerial}]
        $Notebook insert end "$name" -window $newpage
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
		-formatcommand {FormatColumn "% 8.1f"}
	$table columnconfigure 3 -align right \
		-formatcommand {FormatColumn "% 10.0f"}
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
    incr pageSerial
}

#  Process the legal switches associated with the channel commane.
#  These are:
#            -lowalarm value   - Sets the low alarm level for the channel if
#                                omitted, no low alarm level is set for the channel.
#            -hialarm value    - Sets the high alarm level for the channel  if
#                                omitted, no alarm level is set for the channel.
#  For legal switch/value pairs, the value is put into the appropriate lowAlarms or
#  hiAlarms array element.  Illegal switch/value pairs result in a descriptive
#  error throw to the caller.
#
# Parameters:
#   name       - Name of the channel whose switches are being processed.
#   option     - The option (e.g. -lowalarm or -hialarm).
#   value      - A floating point value for the alarm level.
#
proc processChannelSwitch {name option value}  {
    global lowAlarms
    global hiAlarms

    #  Ensure the value is a legal floating point:

    if {[scan $value %f limit] == 0} {
        error "Expected floating point for limit value got $value"
    }
    # Process legal switches and error on illegal:

    switch -exact -- $option {
        -lowalarm {
            set lowAlarms($name) $value
        }
        -hialarm {
            set hiAlarms($name) $value
        }
        default {
            error "Illegal option value: $option must be either -lowalarm or -hialarm"
        }
    }
}


#    channel command
#      The form of the channel command is:
#         channel ?-lowalarm value? ?-highalarm value? name number
#            -lowalarm value   - Sets the low alarm level for the channel if
#                                omitted, no low alarm level is set for the channel.
#            -hialarm value    - Sets the high alarm level for the channel  if
#                                omitted, no alarm level is set for the channel.
#            name              - Is the name to be assigned to the scaler channel.
#            number            - Is the scaler index.  Scaler indices number from 0.
#  Parameters:
#     args       - The command line arguments.
#
set ScalerMap($Fakename) ""          ;# Not likely to be a scaler named this..

proc channel {args} {
    global ScalerMap
    global lowAlarms
    global hiAlarms

    # We first need to pick off the name and channel, based on
    #  The name and channel are always the last two parameters.

    set numParams [llength $args]
    if {$numParams < 2} {
        puts "Too few parameters: channel $args"
        return
    }

    set id      [lindex $args end]
    set name    [lindex $args end-1]

    # short stop any attempts to make a duplicate name:

    if {[array names ScalerMap $name] == $name} {
	puts "Attempt to make a duplicate scaler map of $name to $id ignored"
        return
    }


    # The alarm stuff we can process depends on the number of command parameters:


    incr numParams -2;                          # Remove the name/id from the list.

    #  Legal values for numParams are now 2 and 4:

    if {$numParams == 2} {
        if {[catch {processChannelSwitch $name [lindex $args 0] [lindex $args 1]} msg]} {
            puts "Failed to process command switches channel $args : $msg"
            return
        }
    } elseif {$numParams ==4} {
        if {[catch {processChannelSwitch $name [lindex $args 0] [lindex $args 1]} msg]} {
            puts "Failed to process command switches channel $args : $msg"
            return
        }
        if {[catch {processChannelSwitch $name [lindex $args 2] [lindex $args 3]} msg]} {
            puts "Failed to process command switches channel $args : $msg"
            catch {unset lowAlarms($name)}
            catch {unset hiAlamrs($name)}
            return
        }
    } elseif {$numParams == 0} {
        # No switches nothing to do but perfectly legal.
    } else {
        puts "Invalid number of parameters for command: channel $args"
        puts "command ignored"
        return
    }
    puts "Setting scaler $name $id"
    set ScalerMap($name) $id
}

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

# stripparam  - Indicate that a parameter rate should be on the strip chart.
#               if the strip chart does not exist yet, it is created and configured.
#
# Parameters:
#    channel   - Name of the channel to chart.
#
proc stripparam channel {
    global ScalerMap
    global stripchartChannels
    global stripchartWidget

    # complain but don't do anything if the channel does not exist.

    if {[array names ScalerMap $channel] == ""} {
        puts "stripparam $channel - $channel has not been defined yet"
        return
    }
    # Add the channel to the list and create a vector.
    #
    lappend stripchartChannels $channel
    global $channel
    vector create $channel

    #  If the strip chart does not exist, create and configure it:

    if {$stripchartWidget == ""} {
        set stripchartWidget [createStripChart .]
        configureStripChart
    }
    # Add the vector as a data series:

    set style [selectElementStyle]
    $stripchartWidget element create $channel \
            -xdata timeVector -ydata $channel \
            -smooth linear -symbol ""        \
            -color [lindex $style 0]         \
            -dash  [lindex $style 1]
}
#  stripratio - Creates a stripchart data set that's the
#               ratio of a pair of channels.
# Parameters:
#   numerator    - Name of the numerator channel
#   denominator  - Name of the denominator channel
#
proc stripratio {numerator denominator} {
    global ScalerMap
    global stripchartWidget
    global stripchartRatios
    global timeVector

    # If either channel is not defined, complain and do nothing.

    if {[array names ScalerMap $numerator] ==  ""} {
        puts "stripratio $numerator $denominator : $numerator not yet defined"
        return
    }
    if {[array names ScalerMap $denominator] == ""} {
        puts "stripratio $numerator $denominator : $denominator not yet defined"
        return
    }
    # Add the ratio to the list and create the vector

    set pair [list $numerator $denominator]
    lappend stripchartRatios $pair
    set vecname [ratioName $numerator $denominator]
    global $vecname
    vector create $vecname

    # If the strip chart does not yet exist, create and configure it.

    if {$stripchartWidget == ""} {
        set stripchartWidget [createStripChart .]
        configureStripChart
    }

    # Add the vector as a data series.

    set style [selectElementStyle]
    $stripchartWidget element create $vecname \
            -xdata timeVector -ydata $vecname \
            -smooth linear -symbol ""        \
            -color [lindex $style 0]         \
            -dash  [lindex $style 1]
}
#
#  Configure the stripchart, The args list contains options with potentially values.
#  Options can be any of the following:
#     -log  {0|1}        - Controls whether the y axis of the plot will be log or lin.
#                          0 - linear, 1 - log.  Default is linear.
#     -timeaxis n        - Sets the number of seconds on the x axis.  If not overridden,
#                          defaults to 3600 (1 hour).
# Parameters:
#    args    - the list of command line paramters.
proc stripconfig {args} {
    global xAxisLength
    global yAxisType
    global stripchartWidget

    # Should be an even number of arguments:

    if {[llength $args] % 2} {
        puts "Incorrect number of parameters in stripconfig $args"
        return
    }
    #  Process -option value pairs:

    for {set i 0} {$i < [llength $args]} {incr i 2} {
        set option [lindex $args $i]
        set value [lindex $args [expr $i + 1]]

        switch -exact -- $option {
            -log {
                if {$value} {
                    set yAxisType log
                } else {
                    set yAxisType linear
                }
            }
            -timeaxis {
                if {[scan $value "%d" temp] != 1} {
                    puts "-timeaxis option value must be an integer: stripconfig $args"
                    return
                } else {
                    set xAxisLength $value
                }
            }
        }
    }
    if {$stripchartWidget == ""} {
        set stripchartWidget [createStripChart .]
    }
    configureStripChart

}

#
#

set Notebook [SetupGui ""]



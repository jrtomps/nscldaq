#
#   This gui is intended to be put in a TclServer connected to
#   controlpush;  Controlpush is responsible for maintaining
#   the following arrays:
#     EPICS_DATA (channelname)   - Value of a channel
#     EPICS_UNITS(channelname)   - Units of a channel
#     EPICS_UPDATED(channelname) - Time of last update.
#
#   We take as input a channels.txt file
#   Extract the channel names and 'same line comments',
#   Sort them alphabetically by channel and maintain them
#   in a tktable.
#

package require Tktable
package require BWidget
package require BLT
namespace import ::blt::stripchart
namespace import ::blt::vector

set tableWidget ""
set chartWidget ""
set paneWidget ""


set stripColors {black red green blue cyan magenta brown}
set stripStyles { "" {5 5} {2 2} {5 5 2 2} {5 2} { 5 5 2 5}}
set stripColorIndex 0
set stripStyleIndex 0
set time 0

set autotimeRange 0
set timeRange     2;          #Minutes.

set chartChannels ""

#
#  Select the style for a plot.
#
proc selectStyle {index} {
    global stripColors
    global stripStyles

    set color [lindex $stripColors \
                   [expr $index % [llength $stripColors]]]
    set style [lindex $stripStyles [expr $index / [llength $stripColors]]]

    return [list $color $style]

}
#
#  compareFirst:
#    Simple sorting predicate to compare element 0 of two list
#    elements
#
proc compareFirst {el1 el2} {
    return [string compare $el1 $el2]
}

#
#   Create a the Gui which now consists of
#   a table in a scrolled window.
#   The table starts with 1 row (a row of titles) and columns for:
#   channel name, channel value, channel units, channel comment.
#
proc makeGui {} {
    global paneWidget
    global tableWidget

    set paneWidget [PanedWindow .pwindow -side left]
    set f [$paneWidget add]
    ScrolledWindow   $f.sw
    set tableWidget  [table $f.table -rows 1 -cols 4 -cache 1 \
                                  -titlerows 1  \
                                  -pady 2 -padx 2]
    $tableWidget width 3 15
    $tableWidget set row 0,0 {Name Value Units Comment}
    $f.sw setwidget $tableWidget
    pack $f.sw -fill both -expand 1
}

#
#  Reads a setupfile.  The setup file is read and
#  returned as a  list of pairs:
#  channelname comment
#  sorted by channel name.
proc readSetupFile {filename} {
    set fd [open $filename r]
    set file [read $fd]
    close $fd
    set lines [split $file "\n"]
    foreach line $lines {
        # Skip comment lines.
        if {[string index $line 0] != "#"} {
            set channel [lindex $line 0]
            set comment [lrange $line 1 end]
            lappend channelList [list $channel $comment]
        }
    }
    return $channelList
}
#
#   Processes a channel list returned by readSetupFile e.g.
#   Creates a list of channels to be strip charted.
#   Strip charted channels have comments with the first
#   word "chart"
#
proc getStripchartChannels {channels} {
    set result ""
    foreach channel $channels {
        set comment [lindex $channel 1]
        set keyword [lindex $comment 0]
        if {$keyword == "chart"} {
            lappend result [lindex $channel 0]
        }
    }
    return $result
}

#
#   Fills the gui table with a list of the form created by
#   readSetupFile.
#    For each row we put:
#       col0: name of channel.
#       col1: A label widget that is linked to EPICS_DATA(channelname)
#       col2: A label widget that is linked to EPICS_UNITS(channelname)
#       col3: A comment.
# list:     The list to use to fill the table.
proc fillTable {list} {
    global tableWidget
    foreach channel $list {
        set name [lindex $channel 0]
        $tableWidget insert row end
        set row [$tableWidget cget -rows]
        incr row -1
        $tableWidget set $row,0 [lindex $channel 0]
        $tableWidget set $row,3 [lindex $channel 1]
        label .value$row -textvariable EPICS_DATA($name)
        label .units$row -textvariable EPICS_UNITS($name)

        $tableWidget window configure $row,1 -window .value$row -sticky e
        $tableWidget window configure $row,2 -window .units$row -sticky e
    }
    $tableWidget configure -state disable -colstretchmode last
}
#   Update the strip chart from the chartChannels channels.
#
proc updateStripChart {secs} {
    global chartChannels
    global timeVector
    global time
    global EPICS_DATA
    set ms [expr $secs*1000]


    timeVector append $time
    incr time

    for {set vectorNumber 0} {$vectorNumber < [llength $chartChannels]} {incr vectorNumber} {
        set vectorname channel$vectorNumber
        global $vectorname
        set name [lindex $chartChannels $vectorNumber]
        set value $EPICS_DATA($name)
        if {[string is double -strict $value]} {
            $vectorname append  $value
        }
    }
    after $ms "updateStripChart $secs"
}
#
#  Clear the strip chart:  The time is set back to zero.
#  The vectors are cleared of data:
#
proc clearStripChart {} {
    global timeVector
    global time
    global chartChannels

    set time 0
    timeVector length 0
    for {set n 0} {$n < [llength $chartChannels]} {incr n} {
        set name channel$n
        global $name
    }

}
#
#   Sets the strip chart range.
#   Two globals determine what we should do:
#   if autotimeRange is non-zero, the time range will be set
#   to compress the timescale of the chart as needed to fit all
#   the points.  If not, then timeRange should be a
#   number of minutes of time range (the axes will be in seconds note).
#   As needed, the axis will shift by 10% of range.
proc setStripchartRange {} {
    global autotimeRange
    global timeRange
    global chartWidget
    global time

    if {$autotimeRange} {
        $chartWidget axis configure x -autorange $time
        $chartWidget axis configure x -autorange 0

    } else {
        if {[string is integer -strict $timeRange]} {
            set seconds [expr $timeRange * 60]
            $chartWidget axis configure x \
                -autorange $seconds      \
                -shiftby   [expr $seconds/10]
        } else {
            tk_messageBox -icon error \
               -message "Time range must be an integer number of minutes" \
               -title "Bad int" -type ok
        }
    }
}
#
# Setup the strip chart control panel.  We have a button
# to reset time to 0/clear vectors.
# We have a checkbutton that puts the chart in/or out of
# autoscale and an entry for the time range when the chart
# is not in autoscale.  We also havea 'set' button to make
# the time scale entries take effect.
#
proc setupStripControls {} {
    frame .stripcontrols -bd 2 -relief groove
    button .stripcontrols.clear -text Clear  -command clearStripChart

    frame .stripcontrols.time -bd 2 -relief groove
    checkbutton .stripcontrols.time.auto -variable autotimeRange -text "Auto Range"
    label       .stripcontrols.time.rlabel -text {  Range (min) :}
    entry       .stripcontrols.time.range -textvariable timeRange
    button      .stripcontrols.time.set   -text Set -command setStripchartRange

    pack .stripcontrols.time.auto .stripcontrols.time.rlabel \
         .stripcontrols.time.range .stripcontrols.time.set -side left
    pack .stripcontrols.clear  -side left
    pack .stripcontrols.time   -side right
    pack .stripcontrols -side top -fill x
}

#
#   Setup the stripchart given the list of channels to chart.
#
proc setupStripChart {channels} {
    global paneWidget
    global chartWidget
    global timeVector
    global chartChannels

    set f [$paneWidget add]

    set channels [getStripchartChannels $channels]


    if {[llength $channels] > 0} {
        set chartWidget [stripchart $f.stripchart -height 3i -width 8i]
        pack $chartWidget
        vector create timeVector

        set vectorNumber 0
        foreach channel $channels {
            set name $channel
            global channel$vectorNumber
            set vect [vector create channel$vectorNumber]
            set style [selectStyle $vectorNumber]
            incr vectorNumber
            $chartWidget element create $name \
                    -xdata timeVector -ydata $vect \
                    -smooth linear -color [lindex $style 0] \
                    -symbol ""

            $chartWidget axis configure x -autorange 120 \
                                        -shiftby 12 -title seconds
            lappend chartChannels $name
        }

        updateStripChart 1

    }


}

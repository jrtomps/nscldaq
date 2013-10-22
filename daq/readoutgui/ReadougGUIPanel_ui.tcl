# ReadoutGUIPanel_ui.tcl --
#
# UI generated by GUI Builder Build 107673 on 2005-08-09 08:48:07 from:
#    //homedir/home2/fox/My Documents/DAQDocs/2005a/daq/DirectoryStructure/code/ReadoutGUIPanel.ui
# THIS IS AN AUTOGENERATED FILE AND SHOULD NOT BE EDITED.
# The associated callback file should be modified instead.
#

# Declare the namespace for this dialog
namespace eval ReadoutGUIPanel {}

package require Tk
# ReadoutGUIPanel::ui --
#
#   Create the UI for this dialog.
#
# ARGS:
#   root     the parent window for this form
#   args     a catch-all for other args, but none are expected
#
proc ReadoutGUIPanel::ui {root args} {
    # this handles '.' as a special case
    set base [expr {($root == ".") ? "" : $root}]
    variable ROOT $root
    variable BASE $base
    global   textWidgetHeight

    # Widget Initialization
    labelframe $base._labelframe_1 \
	    -text Host:
    labelframe $base._labelframe_2 \
	    -text {Readout Program}
    labelframe $base._labelframe_4 \
	    -text {Run Number}
    labelframe $base._labelframe_5 \
	    -text Title
    labelframe $base._labelframe_6 \
	    -text {Run Controls}
    labelframe $base._labelframe_8 \
	    -text {Readout Output}
    labelframe $base._labelframe_9 \
	    -text {Elapsed Active Time (d-hh:mm:ss)}
    entry $base.runnumber \
	    -invalidcommand [namespace code [list runnumber_invalidcommand]] \
	    -validate key \
	    -validatecommand {ReadoutGUIPanel::runnumber_validatecommand %P} \
	    -width 6 \
	    -xscrollcommand [namespace code [list runnumber_xscrollcommand]]
    label $base.host
    label $base.path \
	    -width 60
    entry $base.title \
	    -invalidcommand [namespace code [list title_invalidcommand]] \
	    -validatecommand [namespace code [list title_validatecommand %W %d]] \
	    -width 80 \
	    -validate key \
	    -xscrollcommand [namespace code [list title_xscrollcommand]]
    checkbutton $base.recording \
	    -command [namespace code [list recording_command]] \
	    -text {Record }
    button $base.pauseres \
	    -command [namespace code [list pauseres_command]] \
	    -padx 3m \
	    -state disabled \
	    -text Pause
    button $base.startstop \
	    -command [namespace code [list startstop_command]] \
	    -padx 3m \
	    -text Begin -state disabled
    label $base.statusline \
	    -justify right \
	    -text {} \
	    -width 80
    checkbutton $base.timed \
	    -command [namespace code [list timed_command]] \
	    -text {Timed Run}
    spinbox $base.days \
	    -command [namespace code [list days_command]] \
	    -invalidcommand [namespace code [list days_invalidcommand]] \
	    -to 100 \
	    -validatecommand [namespace code [list days_validatecommand]] \
	    -width 3 \
	    -xscrollcommand [namespace code [list days_xscrollcommand]]
    label $base._label_6 \
	    -text -
    spinbox $base.hours \
	    -command [namespace code [list hours_command]] \
	    -invalidcommand [namespace code [list hours_invalidcommand]] \
	    -to 23 \
	    -validatecommand [namespace code [list hours_validatecommand]] \
	    -width 2 \
	    -xscrollcommand [namespace code [list hours_xscrollcommand]]
    label $base._label_7 \
	    -text :
    spinbox $base.minutes \
	    -command [namespace code [list minutes_command]] \
	    -invalidcommand [namespace code [list minutes_invalidcommand]] \
	    -to 59 \
	    -validatecommand [namespace code [list minutes_validatecommand]] \
	    -width 2 \
	    -xscrollcommand [namespace code [list minutes_xscrollcommand]]
    label $base._label_8 \
	    -text :
    spinbox $base.seconds \
	    -command [namespace code [list seconds_command]] \
	    -invalidcommand [namespace code [list seconds_invalidcommand]] \
	    -to 59 \
	    -validatecommand [namespace code [list seconds_validatecommand]] \
	    -width 2 \
	    -xscrollcommand [namespace code [list seconds_xscrollcommand]]
    puts "Height: $textWidgetHeight"
    text $base.output \
	    -xscrollcommand [namespace code [list output_xscrollcommand]] \
	    -yscrollcommand [list $base._scrollbar_4 set] -state disabled \
	-height $textWidgetHeight
    scrollbar $base._scrollbar_4 \
	    -command [list $base.output yview]
    label $base.elapsed \
	    -justify right \
	    -text 0-00:00:00 \
	    -width 12
    menu $base.menu
    menu $base.file \
	    -tearoff 0
    $base.menu add cascade \
	    -label File \
	    -menu $base.file
    $base.file add command \
	    -label New... \
	    -command [namespace code [list filenew_command]]
    $base.file add command \
	    -label Restart... \
	    -command [namespace code [list filerestart_command]]
    $base.file add command \
	    -label Start \
	    -command [namespace code [list filestart_command]]
    $base.file add separator
    $base.file add command \
	    -label Source...    \
	    -command [namespace code [list filesource_command]]
    $base.file add separator
    $base.file add command \
	    -label Exit \
	    -command [namespace code [list fileexit_command]]
    menu $base.scaler \
	    -tearoff 0
    $base.menu add cascade \
	    -label Scalers \
	    -menu $base.scaler
    $base.scaler add command \
	    -label Parameters... \
	    -command [namespace code [list scalerparameters_command]]


    # Geometry Management

    grid $base._labelframe_1 -in $root -row 1 -column 1 \
	    -columnspan 3 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._labelframe_2 -in $root -row 1 -column 5 \
	    -columnspan 7 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._labelframe_4 -in $root -row 3 -column 11 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._labelframe_5 -in $root -row 2 -column 1 \
	    -columnspan 11 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._labelframe_6 -in $root -row 3 -column 1 \
	    -columnspan 9 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._labelframe_8 -in $root -row 5 -column 1 \
	    -columnspan 11 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._labelframe_9 -in $root -row 4 -column 10 \
	    -columnspan 2 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base.runnumber -in $base._labelframe_4 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky ew
    grid $base.host -in $base._labelframe_1 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky ew
    grid $base.path -in $base._labelframe_2 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky ew
    grid $base.title -in $base._labelframe_5 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky ew
    grid $base.recording -in $base._labelframe_6 -row 1 -column 5 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky e
    grid $base.pauseres -in $base._labelframe_6 -row 1 -column 3 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky w
    grid $base.startstop -in $base._labelframe_6 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky w
    grid $base.statusline -in $root -row 11 -column 1 \
	    -columnspan 11 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky ew
    grid $base.timed -in $root -row 4 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base.days -in $root -row 4 -column 3 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base._label_6 -in $root -row 4 -column 4 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base.hours -in $root -row 4 -column 5 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base._label_7 -in $root -row 4 -column 6 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base.minutes -in $root -row 4 -column 7 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base._label_8 -in $root -row 4 -column 8 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky {}
    grid $base.seconds -in $root -row 4 -column 9 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky w
    grid $base.output -in $base._labelframe_8 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky news
    grid $base._scrollbar_4 -in $base._labelframe_8 -row 1 -column 2 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky nsw
    grid $base.elapsed -in $base._labelframe_9 -row 1 -column 1 \
	    -columnspan 1 \
	    -ipadx 0 \
	    -ipady 0 \
	    -padx 0 \
	    -pady 0 \
	    -rowspan 1 \
	    -sticky ew

    # Install the monitor widget:

#    package require spdaqMonitor

#    spdaqMonitor $base.monitor
#   grid x $base.monitor -in $root -column 3 -columnspan 8

    # Resize Behavior
#    grid rowconfigure $root 1 -weight 0 -minsize 2 -pad 0
#    grid rowconfigure $root 2 -weight 0 -minsize 40 -pad 0
#    grid rowconfigure $root 3 -weight 0 -minsize 2 -pad 0
#    grid rowconfigure $root 4 -weight 0 -minsize 40 -pad 0
#    grid rowconfigure $root 5 -weight 1 -minsize 40 -pad 0
#    grid rowconfigure $root 6 -weight 0 -minsize 40 -pad 0
#    grid rowconfigure $root 7 -weight 0 -minsize 40 -pad 0
#    grid rowconfigure $root 8 -weight 0 -minsize 40 -pad 0
#    grid rowconfigure $root 9 -weight 0 -minsize 40 -pad 0
#    grid rowconfigure $root 10 -weight 0 -minsize 97 -pad 0
#    grid rowconfigure $root 11 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $root 1 -weight 0 -minsize 26 -pad 0
    grid columnconfigure $root 2 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $root 3 -weight 0 -minsize 30 -pad 0
    grid columnconfigure $root 4 -weight 0 -minsize 13 -pad 0
    grid columnconfigure $root 5 -weight 0 -minsize 31 -pad 0
    grid columnconfigure $root 6 -weight 0 -minsize 2 -pad 0
    grid columnconfigure $root 7 -weight 0 -minsize 2 -pad 0
    grid columnconfigure $root 8 -weight 0 -minsize 2 -pad 0
    grid columnconfigure $root 9 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $root 10 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $root 11 -weight 0 -minsize 26 -pad 0
    grid rowconfigure $base._labelframe_1 1 -weight 0 -minsize 28 -pad 0
    grid columnconfigure $base._labelframe_1 1 -weight 0 -minsize 40 -pad 0
    grid rowconfigure $base._labelframe_2 1 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_2 1 -weight 0 -minsize 41 -pad 0
    grid rowconfigure $base._labelframe_4 1 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_4 1 -weight 0 -minsize 2 -pad 0
    grid rowconfigure $base._labelframe_5 1 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_5 1 -weight 0 -minsize 53 -pad 0
    grid rowconfigure $base._labelframe_6 1 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_6 1 -weight 0 -minsize 2 -pad 0
    grid columnconfigure $base._labelframe_6 2 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_6 3 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_6 4 -weight 0 -minsize 21 -pad 0
    grid columnconfigure $base._labelframe_6 5 -weight 0 -minsize 16 -pad 0
#    grid rowconfigure $base._labelframe_8 1 -weight 1 -minsize 40 -pad 0
#    grid columnconfigure $base._labelframe_8 1 -weight 0 -minsize 274 -pad 0
#    grid columnconfigure $base._labelframe_8 2 -weight 0 -minsize 2 -pad 0
    grid rowconfigure $base._labelframe_9 1 -weight 0 -minsize 40 -pad 0
    grid columnconfigure $base._labelframe_9 1 -weight 0 -minsize 40 -pad 0
    $root configure -menu $base.menu

    puts "Base: '$base' size: [grid size .]"
}

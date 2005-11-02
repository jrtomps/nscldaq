#!/bin/bash
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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#
#start wish \
exec wish $0 $@

# This file is a simple wrapper for the caenv812Gui widget that is
# software compatible with the obsolete version.  By software compatible,
# we mean that the same configuration files can be used for both versions...
# not that the look and feel are the same.
#
#----------------------------------

set version 1.0
set instanceNumber 0;   # id of next gui widget instance.
set livePanels     0;   # Number of live panels.

set here [file dirname [info script]]
set drivers [file join $here .. ControlDrivers]
set helpdir [file join $here caenv812help]


if {[lsearch -exact $auto_path $here] == -1} {
    set auto_path [concat $here $drivers $auto_path]
}



package require caenv812Gui
package require Iwidgets

#-------------------------------------------------------------------
# processDestroy widget toplevel
#       Called when a widget in the tree under toplevel is destroyed.
#       if the toplevel is destroyed, decrement the live panels
#       global.  If there are no more live panels, exit the application.
#
# Parameters:
#   widget   - Name of the widget being destroyed.
#   toplevel - Name of the toplevel that is the ultimate parent of widget.
#
proc processDestroy {widget toplevel} {
    global livePanels
    if {$widget eq $toplevel} {
        incr livePanels -1
        if {$livePanels <= 0} {
            exit 0
        }
    }
}

#-------------------------------------------------------------------
# getConfigFile
#     Returns a configuration file by prompting for it with a
#     tk_getOpenFile dialog.
# Returns:
#  name of the file or {} if the dialog was cancelled.
#
proc getConfigFile {} {
    set file [tk_getOpenFile                    \
                -defaultextension .cfd          \
                -filetypes [list                \
                            [list {CFD config files} .cfd] \
                            [list {Tcl scripts}      .tcl] \
                            [list {Tk scritps}       .tk]]]
    return $file
}
#--------------------------------------------------------------
# processConfiguration
#      This is called when the configuration has been loaded.
#      The configuration is created as a bunch of global variables:
#         Name        - Name of the device.
#         ModuleBase  - Base Address of the device.
#         Crate      - The vme crate.
#         Thresholds(0-15)  - initial values of the thresholds.
#         WidthLow/WidthHigh  - Initial values of the widths.
#         DeadTimeLow/DeadTimeHigh - Initial values of the deadtimes
#         mask_arr(0-15) enables mask bits.
#     these device settings are marshalled into a property list
#     that is global and named deviceProperties
#     Any missing configuration items (Name, Crate) that can be are
#     given default values.
#     If Module Base is not supplied, that's an error.
#
proc processConfiguration {} {

        # Deal with defaults

    if {[info globals ModuleBase] == ""} {
        tk_messageBox -icon error -title "Missing base" -type ok \
            -message {The configuration file does not specify the module base address!}
        exit -1
    }
    if {[info globals Name] == ""} {
        uplevel #0 set Name {Cfd Module}
    }
    if {[info globals Crate] == ""} {
        uplevel #0 set Crate 0
    }
        # Deal with property lists.

    set ::deviceProperties [list]
    global deviceProperties


    # Thresholds:

    if {[info globals Thresholds] != ""} {
        global Thresholds
        for {set i 0} {$i < 16} {incr i} {
            if {[array names Thresholds $i] != ""} {
                set property [format "threshold%d" $i]
                set value   $Thresholds($i)
                lappend deviceProperties [list $property $value]
            }
        }
    }
    # Width:
    #
    if {[info globals WidthLow] != ""} {
        global WidthLow
        lappend deviceProperties [list width0 $WidthLow]
    }
    if {[info globals WidthHigh] != ""} {
        global WidthHigh
        lappend deviceProperties [list width1 $WidthHigh]
    }
    # Deadtimes:
    #
    if {[info globals DeadTimeLow] != ""} {
        global DeadTimeLow
        lappend deviceProperties [list deadtime0 $DeadTimeLow]
    }
    if {[info globals DeadTimeHigh] != ""} {
        global DeadTimeHigh
        lappend deviceProperties [list deadtime1 $DeadTimeHigh]
    }
    # The mask is built up of individual array elements.
    #
    set mask 0
    if {[info globals mask_arr] != ""} {
        global mask_arr
        foreach element [array names mask_arr] {
            if {[string is integer -strict  $element]} {
                if {$mask_arr($element)} {
                    set mask [expr {$mask | (1 << $element)}]
                }
            }
        }
    }
    lappend deviceProperties [list enables $mask]

    # The majority threshold:

    if {[info globals Majority] != ""} {
        global Majority
        lappend deviceProperties [list majority $Majority]
    }

}
#---------------------------------------------------------------------------
# newGui file
#       Reads a configuration file and creates a new gui for it.
# Parameters:
#  file - name of the file to read.
#
proc newGui file {
       uplevel #0 source $file
        processConfiguration
        global Name
        global Module
        global Crate
        global ModuleBase
        global deviceProperties
        global instanceNumber

        set toplevel .top$instanceNumber
        incr instanceNumber
        toplevel $toplevel
        buildGui $toplevel $Name $ModuleBase $Crate $deviceProperties
}
#---------------------------------------------------------------------------
#  newConfiguration
#       Prompts for and reads in a new configuration. The new configuration
#       creates a new toplevel gui to control that device (the assumption is
#       that configuration represents a cfd not yet being controlled.
#
proc newConfiguration {} {
    set file [getConfigFile]
    if {$file != ""} {
        newGui $file
    }
}
#-------------------------------------------------------------------------
#   readConfiguration base
#       Prompts for and reads a configuration file.  The device properties
#       are loaded into the device.  Any properties relating to the adressing
#       the module are ignored.
# Parameters:
#    base   - The base widget used to locate the cfd controller which is
#               $base.controlpanel
#
proc readConfiguration {base} {
    catch {uplevel #0 unset Crate};             # Needed to let crate number default.
    set panel $base.controlpanel
    set file [getConfigFile]
    if {$file != ""} {
        uplevel #0 source $file
        processConfiguration
        global deviceProperties
        $panel setProperties $deviceProperties
    }
}
#--------------------------------------------------------------------------
# writeConfiguration base
#     Prompts for a file to which we can write the current configuration.
#     The entire configuration, including device name and addressing information
#     are written so that the file that is written can be used to configure
#     the gui completely in the future.
# Parameters:
#   base   - The base widget (canonicalized for .) used to locate the
#            control panel snit::widget which is $base.controlpanel
#
proc writeConfiguration {base} {
    set panel $base.controlpanel
    set properties [$panel getProperties]
    set name  [$panel cget -name]
    set base  [$panel cget -base]
    set crate [$panel cget -crate]

    set file [tk_getSaveFile -defaultextension .cfd       \
                             -parent           $panel      \
                             -filetypes [list             \
                                    [list {CFD config files} .cfd] \
                                    [list {Tcl Scripts}      .tcl] \
                                    [list {Tk Scripts}       .tk] ]]
    if {$file != ""} {
        if {[catch {open $file w} fd]} {
            tk_messageBox -icon error -title {Create Failed} \
                          -message "Could no open $file : $fd"
        } else {
            # First module id/addressing:

            puts $fd "# CFD configuration file written [clock format [clock seconds]]"
            puts $fd "set Name       $name"
            puts $fd "set ModuleBase $base"
            puts $fd "set Crate      $crate"

            # Now the property list

            foreach element $properties {
                set property [lindex $element 0]
                set value    [lindex $element 1]

                switch -glob $property {
                    threshold* {
                        scan $property "threshold%d" channel
                        puts $fd "set Thresholds($channel) $value"
                    }
                    width* {
                        scan $property "width%d" bank
                        if {$bank == 0} {
                            puts $fd "set WidthLow $value"
                        } else {
                            puts $fd "set WidthHigh $value"
                        }
                    }
                    deadtime* {
                        scan $property "deadtime%d" bank
                        if {$bank == 0} {
                            puts $fd "set DeadTimeLow $value"
                        } else {
                            puts $fd "set DeadTimeHigh $value"
                        }
                    }
                    majority {
                        puts $fd "set Majority $value"
                    }
                    enables {
                        for {set i 0} {$i < 16} {incr i} {
                            if {($value & (1 << $i)) != 0} {
                                puts $fd "set mask_arr($i) 1"
                            } else {
                                puts $fd "set mask_arr($i) 0"
                            }
                        }
                    }
                }
            }
            close $fd
        }
    }
}
#---------------------------------------------------------------------------
#  closeGui    top
#        Close the GUI on which this was invoked.  We give the user a chance to
#        save the state first.
# Parameters:
#   top   - The toplevel widget to destroy.
#
proc closeGui {top} {
    set answer [tk_messageBox -type yesno -icon question -title {Save configuration?} \
                -parent $top \
                -message {You are closing this control panel.  Do you want to save the settings first?}]
    if {$answer eq "yes"} {
        if {$top eq "."} {
            set basewidget ""
        } else {
            set basewidget $top
        }
        writeConfiguration $basewidget
    }
    destroy $top
}
#--------------------------------------------------------------------------
# exitGui
#     Ensure the user really wants to exit the gui and then do it if so.
#
proc exitGui {} {
    set answer [tk_messageBox -type yesno -icon warning -title {Really exit?} \
        -parent [focus]                                                       \
       -message {You are exiting the application.  Any currently open control panels will be closed.  Are  you sure?}]
    if {$answer eq "yes"} {
        exit 0
    }

}

#-------------------------------------------------------------------------
#  displayHelp
#       If the hypertext help widget is not already displayed, bring it up.
#       If it is displayed, do a deiconify to be sure it's visible.
proc displayHelp {} {
    global helpdir
    if {![winfo exists .help]} {
        ::iwidgets::hyperhelp .help  -helpdir $helpdir   \
            -topics {overview gui config}
        .help showtopic overview
    }
    wm deiconify .help
    focus .help
}

#--------------------------------------------------------------------------
#  displayAbout
#      Displays a help->about dialog box that shows what we do and what our
#      current version is.. as well as who's responsible for this.
#
proc displayAbout {} {
    global version
    set    about "CAENV812 Gui version $version\n"
    append about "Author:  Ron Fox\n"
    append about "(c) Michigan State University All Rights reserved 2005\n"
    append about "    Licensed under the GNU Public License (GPL)\n"
    append about "    See: http://www.gnu.org/licenses/gpl.txt\n"
    append about "Report all errors at: http://daqbugs.nscl.msu.edu"

    tk_messageBox -icon info -title {About} -message $about

}

#---------------------------------------------------------------------------
#
# buildGui toplevel name base crate properties
#       Builds the gui.  The gui consists of a menu-bar with a File and Help
#       menu and the cfdv812 gui.
#       Once built, the property list is loaded into the gui.
# Parameters:
#    toplevel   - The name of a toplevel in which to put the GUI (must
#                 already exist).
#    name       - Name to give the GUI (wm title used to set this).
#    baseaddress- Module base address
#    crate      - Module VME crate.
#    properties - Property list containing initial properties to load into
#                 the widget.
#
proc buildGui {toplevel name baseaddress crate properties} {
    global livePanels

    # If the toplevel is . appending additional widgets to it is a special case.

    if {$toplevel eq "."} {
        set base ""
    } else {
        set base $toplevel
    }
    #  Title the top level:

    wm title $toplevel $name

    #  Now create the menu bar and attach it to the top level.

    menu $base.menu
    menu $base.menu.file -tearoff 0
    menu $base.menu.help -tearoff 0
    $base.menu add cascade -label {File} -menu $base.menu.file
    $base.menu add cascade -label {Help} -menu $base.menu.help
    $toplevel configure -menu $base.menu

    # Build the file menu:

    $base.menu.file add command -label {New...}  -command [list newConfiguration]
    $base.menu.file add separator
    $base.menu.file add command -label {Read...} -command [list readConfiguration $base]
    $base.menu.file add command -label {Save...} -command [list writeConfiguration $base]
    $base.menu.file add separator
    $base.menu.file add command -label {Close}   -command [list closeGui $toplevel]
    $base.menu.file add command -label {Exit...} -command [list exitGui]

    # build the Help menu.

    $base.menu.help add command -label {Topics...} -command displayHelp
    $base.menu.help add command -label {About...}  -command displayAbout


    # Add in the control panel and load its initial configuration:

    cfdv812Gui $base.controlpanel -crate $crate -name $name -base $baseaddress
    pack $base.controlpanel -fill both -expand 1
    $base.controlpanel setProperties $properties

    # Do the book keeping needed to exit the application if all GUI's are
    # killed off:

    incr livePanels
    bind  $toplevel <Destroy> [list processDestroy %W $toplevel]

}

#---------------------------------------------------------------------------
# Entry point.
#    If necessary, prompt for a configuration file.
#
wm withdraw .
if {[info vars argv] == ""} {
    set argv [list]
}

if {[llength $argv] == 0} {
    set configFile [getConfigFile]
    # Ensure the file exists and is readable... issuing appropriate errors if not
    #
    if {![file exists $configFile]} {
        tk_messageBox -title {No such file} -icon error -type ok \
            -message "The file $configFile does not exist.  Please chose a configuration file that exists."
        exit -1
    }
    if {![file readable $configFile]} {
        tk_messageBox -title {Not readable} -icon error -type ok \
            -message "The file $configFile is not readable.  Please check the permissions on the file."
        exit -1
    }
    newGui $configFile
} else {
    foreach file $argv {
        newGui $file
    }
}





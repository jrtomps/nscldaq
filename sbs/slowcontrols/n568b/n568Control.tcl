#!/bin/bash

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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#
#start wish \
exec tclsh $0 $@

# Packages:

set here [file dirname [info script]]
set drivers [file join $here .. TclLibs]
set wd [pwd]
cd $drivers
set drivers [pwd]
cd $wd

if {[lsearch -exact $auto_path $here] == -1} {
    set auto_path [concat $drivers $auto_path]
}
package require Tk
package require Iwidgets
package require n568Panel


#  Global data:

set Version         1.0
set panels          0
set activePanels    0
set helpdir         [file join $here $drivers data n568];      # Help files live here.
set configFileTypes [list                               \
                        [list {n568 configurations} .n568] \
                        [list {configuration files} .cfg]  \
                        [list {Tcl Scripts}         .tcl]  \
                        [list {Tk Scripts}          .tk]]


#-----------------------------------
# topLevelName
#      Returns a new toplevel name for a panel.
#
proc topLevelName {} {
    global panels

    append name .n568b $panels
    incr panels

    return $name
}
#-----------------------------------
# onDestroy  toplevel widget
#      Called when a widget is destroyed.
#      if the widget is a top level, then the activePanels global
#      is decremented and, if zero, the program exits.
# Parameters:
#   toplevel    - The name of the toplevel of widget.
#   widget      - the dying widget
# NOTE:
#    We need to do things this way because establishing a destroy binding
#    on a toplevel also fires it for all the toplevel's kids.
proc onDestroy {widget toplevel} {
    global activePanels
    if {$widget == $toplevel} {
        incr activePanels -1
        if {$activePanels == 0} {
            exit
        }
    }
}
#-----------------------------------
# promptConfigFile
#      Prompts the user for a configuration file.  see comments at
#      processConfiguration for more information about the format
#      of a file. Config files are supposed to have extensions
#      like .n568, .cfg, .tcl, or .tk
#
proc promptConfigFile {} {
    global configFileTypes
    set file [tk_getOpenFile -defaultextension .n568                        \
                             -filetypes  $configFileTypes                   \
                             -title {Choose Configuration File}]

}
#-------------------------------------
#  processConfiguration file
#       sources the configuration file into this proc context and
#       from it produces a list that describes the configuration.
#       The configuration file is really just a tcl script that sets
#       a bunch of variables we are interested in.  The variables we care
#       about are:
#              Identification variables
#        name    -Provides a human name for the module.
#        crate   -The crate number of the caennet controller.
#        base    -The base address of the caennet controller.
#        nodeid  -The node on caennet of the shaper module.
#              Property list variables.
#        outconfig(i)   - The output configuration for channel i: 0 - direct 1 - inverted
#        outpolarity(i) - The output polarity for channel i: 0 +, 1 -.
#        shapetime(i)   - The shaping time for channel i:
#                            0 - 200ns, 1 - 1usec, 2 - 3usec, 3 - 6usec.
#        coarsegain(i)  - The coarse gain of channel i:
#                          1 x1, 2 x2, 4 x 4, 8 x 8 ... 128 x 128.
#        finegain(i)    - The fine gain of channel i in the range [0, 255].
#        offset         - The common offset in mv.
#
# Parameters:
#    file - a relative or absolute path to the file.
# Returns:
#   A 5 element list containing the name, controller crate, controller
#    base address, shaper node, and the propertylist represented by
#   the rest of the file.   Note that configuration files for this
#   control panel ignore the multiplexor settings (muxenable and muxsel).
#
proc processConfiguration file {
    # Defaults:

    set name {Anonymous}
    set crate 0
    set base  "invalid"
    set nodeid "invalid"

    source $file

    # produce the properties list:

    set properties [list]
    if {[info locals offset] != [list]} {
        lappend properties [list offset $offset]
    }
    #  Now the ones that are good for each channel:

    foreach source [list outconfig outpolarity shapetime coarsegain finegain polezero] \
            dest   [list invert    polarity    shapetime coarse     fine polezero] {
        if {[info locals $source] != [list]} {
            for {set i 0} {$i < 16} {incr i} {
                if {[array names $source $i] != [list]} {
                    lappend properties [list [format %s%d $dest $i] [set ${source}($i)]]
                }
            }
        }
    }
    # Return the final list.

    return [list $name $crate $base $nodeid $properties]
}

#-------------------------------------
#  newPanel file
#       Create a new toplevel panel given a configurationfile.
#       This proc contains the code to deal with files that don't exist.
# Parameters:
#   file   - The name of the configuration file on which the panel should be
#            made.
#
proc newPanel file {
    global activePanels
    if {![file exists $file]} {
        tk_messageBox -title {No such file} -icon error       \
            -message "$file can not be found"
        return
    }
    if {![file readable $file]} {
        tk_messageBox -title {Can't read file} -icon error          \
            -message "$file exists but is not readable by this login"
        return
    }

    set info [processConfiguration $file]
    set name  [lindex $info 0]
    set crate [lindex $info 1]
    set base  [lindex $info 2]
    set node  [lindex $info 3]
    set properties [lindex $info 4]

    # Ensure the base and node have been supplied:

    if {$base eq "invalid"} {
        tk_messageBox -title {No base} -icon error \
            -message {The configuration file does not specify the required 'base' variable}
        return
    }
    if {$node eq "invalid"} {
        tk_messageBox -title {No node} -icon error \
            -message {The configuration file does not specify the required 'nodeid' variable}
        return
    }

    # Now construct a new toplevel and put an n568Panel into it and a menu bar on it.

    set topname [topLevelName]
    toplevel $topname -menu $topname.menu

    menu $topname.menu
    menu $topname.menu.file -tearoff 0
    menu $topname.menu.help -tearoff 0

    $topname.menu add cascade -label File -menu $topname.menu.file
    $topname.menu add cascade -label Help -menu $topname.menu.help

    $topname.menu.file add command -label New... -command fileNew
    $topname.menu.file add separator
    $topname.menu.file add command -label Load... -command [list fileLoad $topname.panel]
    $topname.menu.file add command -label Save... -command [list fileSave $topname.panel]
    $topname.menu.file add separator
    $topname.menu.file add command -label Close   -command [list fileClose $topname]
    $topname.menu.file add command -label Quit    -command fileExit

    $topname.menu.help add command -label About...  -command helpAbout
    $topname.menu.help add command -label Topics... -command helpTopics

    $topname configure -menu $topname.menu

    wm title $topname $name
    n568Panel $topname.panel -controllercrate $crate -controllerbase $base -node $node -name $name
    $topname.panel setProperties $properties
    pack $topname.panel -fill both -expand 1

    incr activePanels
    bind $topname <Destroy> [list onDestroy $topname %W]
}

#---------------------
# helpAbout
#     Displays the help->about dialog box.
#
proc helpAbout {} {
    global Version
    tk_messageBox -icon info -title {About} \
        -message "n568Control V $Version\n\
Author: Ron Fox\n\
(c) Michigan State University 20005\n\
    All rights reserved\n\
    licensed for distribution under the gpl"

}
#---------------------
# helpTopics
#     If the help widget is not being displayed, we
#     create it.
#
proc helpTopics {} {
    global helpdir
    if {![winfo exists .help]} {
        ::iwidgets::hyperhelp .help -helpdir $helpdir -topics {overview gui config}
        .help showtopic overview

    }
    wm deiconify .help

}
#---------------------
# fileNew
#       Prompts the user for a configuration file.  If one is supplied,
#       it is used to create a new panel.
#
proc fileNew {} {
    set name [promptConfigFile]
    if {$name ne ""} {
        newPanel $name
    }
}
#---------------------
# fileLoad panel
#        Loads a configuration file into the control panel
#        specified.  Note that the identification configuration is
#        ignored, only settings are restored.
# Parameters:
#   panel - The widget name of the control panel to load.
proc fileLoad widget {
    set top [winfo parent $widget]
    set oldcursor [$top cget -cursor]
    $top configure -cursor watch
    set name [promptConfigFile]
    if {$name ne ""} {
        set info [processConfiguration $name]
        set properties [lindex $info 4]
        $widget setProperties $properties
    }
    $top configure -cursor $oldcursor
}
#----------------------
# writeProperties fd properties
#       Writes the property list of the device (settings) to a
#       configuration file.
# Parameters:
#   fd         - File open to which the data are written.
#   properties - The property list (name value pairs).
#
proc writeProperties {fd properties} {
    foreach property $properties {
        set name   [lindex $property 0]
        set value  [lindex $property 1]
        switch -glob $name {
            coarse* {
                scan $name "coarse%d" channel
                set  name  "coarsegain($channel)"
            }
            fine* {
                scan $name "fine%d" channel
                set  name  "finegain($channel)"
            }
            polezero* {
                scan $name "polezero%d" channel
                set  name   "polezero($channel)"
            }
            invert* {
                scan $name "invert%d" channel
                set   name  "outconfig($channel)"
                if {$value eq "inverted"} {
                    set value 1
                } else {
                    set value 0
                }
            }
            polarity* {
                scan $name "polarity%d" channel
                set  name  "outpolarity($channel)"
                if {$value eq "-"} {
                    set value 1
                } else {
                    set value 0
                }
            }
            shapetime* {
                scan $name "shapetime%d" channel
                set  name  "shapetime($channel)"
            }
            offset {

            }
            default {
            }
        }
        puts $fd "set $name $value"
    }
}
#----------------------
# fileSave panel
#       Saves a configuration file that can be used
#       to reproduce the control panel in its current state.
#       This includes module identification and addressing information
#       as well as settings.
# Parametrs:
#   panel   - The widget of the control panel to save.
#
proc fileSave panel {
    global configFileTypes
    set top [winfo parent $panel]
    set oldcursor [$top cget -cursor]
    $top configure -cursor watch

    set file [tk_getSaveFile  -title "Choose save file"       \
                              -defaultextension .n568         \
                              -filetypes $configFileTypes]

    if {$file != ""} {
        if {[catch {open $file w} fd] == 0} {
            puts $fd "# N568b configuration file written by n568Control.tcl [clock format [clock seconds]]"

            # Fetch out the id information and save it:

            puts $fd "set name    [$panel cget -name]"
            puts $fd "set crate   [$panel cget -controllercrate]"
            puts $fd "set base    [$panel cget -controllerbase]"
            puts $fd "set nodeid  [$panel cget -node]"

            # now the settings...which come from the propertylist:

            set properties [$panel getProperties]
            writeProperties $fd $properties


            close $fd

        } else {
            tk_messageBox -title "File unwritable" -icon error            \
                -message "$file could not be open for write: $fd"
        }
    }
    $top configure -cursor $oldcursor
}
#---------------------------------
# fileClose topwidget
#         Closes the control panel it's on.  The destroy callbacks will
#         automatically close our application when the last one is closed.
#
proc fileClose topwidget {
    set answer [tk_messageBox -type okcancel -title "Are you sure?" -icon question \
                    -message "If you have not saved the settings for this control panel, the current settings will be lost.  Ok to close?"]
    if {$answer eq "ok"} {
        destroy $topwidget
    }
}
#---------------------------------
# fileExit
#       Exits the program.
#
proc fileExit {} {
    set answer [tk_messageBox -type okcancel -title "Are you sure?" -icon warning \
                 -message "Exiting the program will close all shaper control panels and lose any \
settings that are not yet saved.  Ok to exit?"]
    if {$answer eq "ok"} exit
}
#------------------------------- Entry point -----------------------------

#Usage:
#    n568Control.tcl ?configfile? ....
#
# This application implements a multipaged control panel for the N568b
# shaping amplifier.  Each amplifier occupies a new toplevel widget.
# The amplifier configuration is held in configuration files that
# describe the amplifier in terms of tcl variables and arrays (see
# processConfiguration for more information about this).
# The end result is a control panel that is a functional (but not gui) superset
# of the old control panel for shapers in the config directory.
#
#  If no configuration file is specified, the user is prompted for one.
#


wm withdraw .

set configfiles $argv
if {[llength $argv] == 0} {
    set configfiles [promptConfigFile]
}

if {[llength $configfiles] == 0} {
    tk_messageBox -icon info -title {No configs} \
            -message {Since you insist on not having any configuration files, I will exit}
    exit 0
}


foreach file $configfiles {
    newPanel $file
}



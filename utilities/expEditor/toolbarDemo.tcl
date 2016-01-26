#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file toolbarDemo.tcl
# @brief Demonstrate/exercise toolbars and tools.
# @author Ron Fox <fox@nscl.msu.edu>
#

set here [file dirname [info script]]

if {[array names env DAQROOT] ne ""} {
    
    # DAQROOT is defined so we can locate TclLibs relative to that.
    
    lappend auto_path [file join $env(DAQROOT) TclLibs]
}  else {
    # Assume we're in DAQBIN
    
    lappend auto_path [file normalize [file join $here .. TclLibs]]
}




package require toolbar
package require ringBufferObject
package require stateProgram
package require Service
package require objectInstaller
package require tool
package require connectorObject
package require eventBuilderObject
package require dataSourceObject
package require connectorInstaller

package require varmgr
package require stateclient
package require daqservices
package require vardbEventBuilder
package require vardbringbuffer

package require svcSerializer
package require ringSerializer
package require stateProgramSerializer

# Global variables:

set dbFile ""


##
# createDbFile
#   Create a database file with all the schema installed.
#
# @param dbFile - path to the file.
#
proc createDbFile dbFile {
    varmgr::create $dbFile
    set uri file://[file normalize $dbFile]
    exec $::env(DAQBIN)/vardbsh $uri <  [file join $::env(DAQBIN) MakeRunControl.tcl]
    
    ::nscldaq::services svcapi $uri
    svcapi  create
    ::nscldaq::services -delete svcapi
    
    ::nscldaq::evb  create evbapi $uri
    evbapi createSchema
    ::nscldaq::evb  destroy evbapi
    
    ::nscldaq::vardbringbuffer create rbapi $uri
    rbapi createSchema
    ::nscldaq::vardbringbuffer destroy rbapi
    
}

# Make the tool bar:

toolbar .t -width 60 -height 512

# Make the target canvas, bind the toolbar to it and layout the ui:

canvas .c -width 512 -height 512
.t configure -target .c
grid .t .c -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 1 -weight 1

# Create and add some tools:

set os [ObjectInstaller %AUTO%]

tool ring [RingBufferObject %AUTO%] $os
.t add ring

tool statePgm [StateProgram %AUTO%] $os
.t add statePgm

tool eventbuilder [EventBuilderObject %AUTO%] $os
.t add eventbuilder
tool dsource      [DataSourceObject %AUTO%] $os
.t add dsource

tool service [Service %AUTO%] $os
.t add service



# Connector

set cs [ConnectorInstaller %AUTO%]
$os configure -deletecmd [list $cs uninstall %O %I %W]
$os configure -installcmd [list $cs newObject %O %I %W]
tool knect [ConnectorObject %AUTO%] $cs;   # Need new installer
.t add knect

#  Establish the file if provided:



if {[llength $argv] > 0} {
    set dbFile [lindex $argv 0]
    
    #  If necessary, create the database file:
    
    if {![file exists $dbFile]} {
        createDbFile $dbFile     
    }
}


#----------------------------------------------------------------------------
#
#  Menu processing:
#


##
# showAbout
#   Show about help.
#
proc showAbout {} {
    tk_messageBox -parent . -icon info -title About: -type ok \
        -message "Experiment editor \nv1.0, \nRon Fox"
}

##
# saveState
#   Save the editor state to the database.
#   If there's no dbFile, one is prompted for and, if necessary, created/initialized.
#
proc saveState {} {
    if {$::dbFile eq ""} {
        set ::dbFile \
            [tk_getSaveFile  \
                -defaultextension .db -confirmoverwrite 0 -parent . \
                -filetypes [list\
                    [list "Database File" .db]              \
                    [list "All files"     *]                \
                ]                                           \
                -title {Save editor state to:}
        ]
        if {$::dbFile eq ""} return;               # No new file selected.
        
        #  Can select an existing file ..
        
        if {![file exists $::dbFile] } {
            createDbFile $::dbFile
        }    
    }
    #  Code to do the actual save goes here:
    set uri  file://[file normalize $::dbFile]
    ::Serialize::serializeServices $uri [$::cs listObjects .c service]
    ::Serialize::serializeRings    $uri [$::cs listObjects .c ring]
    ::Serialize::serializeStatePrograms $uri [$::cs listObjects .c state-program]
}

##
# exitProgram
#    Exit the program:
#    -    If there's not a database file, do saveState to prompt for one..and
#         save if the user selects one.
#    -    If there's a database file, ask if we want to save data to it.
#         If so -> saveState
#    -    Prompt for exit confirmation. etc. etc.
#

proc exitProgram {} {
    if {$::dbFile eq ""} {
        saveState
    } else {
        set saveOk [tk_messageBox \
            -parent . -title {Save State?} -icon question  -type yesnocancel \
            -default yes -message {Save current state?}                        \
        ]
        if {$saveOk eq "cancel"} return
        if {$saveOk eq "yes"} saveState
    }
    #  Now that the save etc. is out of the way are we sure we want to exit?
    
    set exitOk [tk_messageBox \
        -parent . -title {Exit?} -icon question -type yesno -default no \
        -message {Are you sure you want to exit the editor?}            \
    ]
    if {$exitOk eq "yes"} exit
}

#  Establish the menubar and its menus:

tk::classic::restore menu

menu .m
. configure -menu .m
.m add cascade -label File -menu .m.file
.m add cascade -label Help -menu .m.help


# TODO:  Hook the destroy of saveState so there's a chance to save stuff.
# TODO:  Know if there have been changes and make some of this save stuff
#        conditional on changes.

menu .m.file -tearoff 0
.m.file add command -label Save -command saveState
.m.file add separator
.m.file add command -label Exit... -command exitProgram

menu .m.help -tearoff 0
.m.help add command -label About... -command showAbout

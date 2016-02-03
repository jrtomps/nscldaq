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
package require evbSerializer
package require dsourceSerializer

package require serviceValidator
package require ringValidator
package require stateProgramValidator
package require eventBuilderValidator
package require dataSourceValidator

package require dialogWrapper
package require checklist

# Global variables:

set dbFile ""

##
#  initDbFile
#    Initialize the chunks of the database file we need
#
# @param dbFile - name of the database file
#
proc initDbFile {dbFile} {
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
##
# createDbFile
#   Create a database file with all the schema installed.
#
# @param dbFile - path to the file.
#
proc createDbFile dbFile {
    varmgr::create $dbFile
    initDbFile     $dbFile    
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
# validateConfiguration
#   Validate that all of the objects at least look reasonable.
#   - If validation is ok a tk_messageBox tells you the config looks good.
#   - If validation fails a checklist non-modal dialog is popped up to show
#     the user what they have to fix.
#
# @note regardless, any prior dialog is destroyed.
#
proc validateConfiguration {} {
    destroy .validations

    set messages [list];			# List of things to do.
    
    set svcs [$::cs listObjects .c service]
    set messages [concat $messages [::Validation::validateServices $svcs]] 

    set rings [$::cs listObjects .c ring]
    set messages [concat $messages [::Validation::validateRings $rings]]

    set sps [$::cs listObjects .c state-program]
    set messages [concat $messages [::Validation::validateStatePrograms $sps]]

    set evbs [$::cs listObjects .c eventbuilder]
    set messages [concat $messages [::Validation::validateEventBuilders $evbs]]

    set ds [$::cs listObjects .c datasource]
    set messages [concat $messages [::Validation::validateDataSources $ds]]

    if {[llength $messages] == 0} {
	tk_messageBox -parent . -title "validates ok" -type ok -icon info \
	    -message {Validation successful}
    } else {
	toplevel .validations
	NonModalDialogWrapper .validations.d \
	    -okcommand [list destroy .validations] -cancelcommand [list destroy .validations]
	set cl [checklist [.validations.d controlarea].cl]
	foreach msg $messages {
	    $cl addItem $msg
	}
	.validations.d configure -form $cl
	pack .validations.d
	
    }
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
    ::Serialize::serializeEventBuilders $uri [$::cs listObjects .c eventbuilder]
    ::Serialize::serializeDataSources $uri [$::cs listObjects .c datasource]
    

}
##
# installObjects
#   Used to install a set of objects given a template and coordinates:
#
# @param objects - list of dicts containing the keys:
#                  * object - object to clone into the canvas.
#                  * x,y    - Final position on the canvas.
# @note as a side effect, the objects that are the source of the clone get
#       destroyed.
#
proc installObjects objects {
    foreach item $objects {
        set o [dict get $item object]
        set x [dict get $item x]
        set y [dict get $item y]
        
        set newO [$::os install $o [dict create canvas .c x  $x  y $y] .c]
        $newO moveto $x $y
        $o destroy
    }
}
##
# uriToRingIdx
#
#   Converts a ring uri (form tcp://host/ringname) to a string of the form;
#   ringname@host.
#
# @param uri
# @return string - see above.
#
proc uriToRingIndex {uri} {
    set uriParts [split $uri /]
    
    # The parts we want are the last and next to last elements (name/host repsectively)
    
    set name [lindex $uriParts end]
    set host [lindex $uriParts end-1]
    
    return $name@$host
}
##
# connectObjects
#   Connects a pair of objects together (part of restoring from file)
#
# @param from - object the connection originates at.
# @param to   - object the connection terminetes at
#
# @note recall that connections are directional.
#
proc connectObjects {from to } {
    
    $::cs connect .c $from $to
    
}
##
#  ringsToIndexList
#    Converts the list of ringbuffers to a list that can be used with array set
#    to produce an array of ring objects indexed by ringname@host
#
# @param rings - list of input ring objects.
# @return list - ring1@host1 ringobject ...
#
proc ringsToindexList {rings} {
    set result [list]
    foreach ring $rings {
        set p [$ring getProperties]
        set name [[$p find name] cget -value]
        set host [[$p find host] cget -value]
        set ringIndex $name@$host
        lappend result $ringIndex $ring
    }
    
    return $result
}
##
# connectStateProgramsToRings
#    Re-connects state programs back to the in/out rings they were originally
#    connected to.
#
# @param statePrograms - list of state program objects.
# @param rings         - list of ring objects.
#
proc connectStateProgramsToRings {statePrograms rings} {
    
    #  Toss the rings up into a name@host indexed array for O(1) lookup:
    array set ringArray [ringsToindexList $rings]

    #  Connect each state Programto the ring(s) it used to be connected with
    #  remember connections require two connect calls one for the source
    #  and one for the target.  Note also that the ring specifications are:
    #
    #  Output Ring - A ring name for the host the state propgram is in.
    #  Input Ring  - A ring URI of the form: tcp://hostname/ringname
    
    foreach sp $statePrograms {
        set p [$sp getProperties]
        set host [[$p find host] cget -value];   # In case we have an out ring.
        
        set oring [[$p find "Output Ring"] cget -value]
        if {$oring ne ""} {
            # Reconnect output ring:
            
            set ringIdx $oring@$host;           # Index in array:
            connectObjects $sp $ringArray($ringIdx)
        }
        
        set iring [[$p find "Input Ring"] cget -value]
        if {$iring ne ""} {
            set ringIdx [uriToRingIndex $iring]
            connectObjects  $ringArray($ringIdx) $sp
        }
    }
}
##
# connectDsToInRings
#    Connect data sources to their input rings.
#
# @param dataSources - list of data source objects.
# @param rings       - list of ringbuffer objectgs.
#
proc connectDsToInRings {dataSources rings} {
    
     array set ringArray [ringsToindexList $rings]
     
    # the input rings of data sources are uris... these have to be converted
    # to name@host strings.
    #
    
    foreach source $dataSources {
        set p [$source getProperties]
        set inring [[$p find ring] cget -value]
        if {$inring ne ""} {
            set index [uriToRingIndex $inring]
            connectObjects $ringArray($index) $source
        }
    }
}

##
# restoreConnections
#    Restore connections between objects as part of restoring the editor
#    state from a database file:
#
#  @param uri - URI specifying how to connect to the database if needed.
#
proc restoreConnections uri {
    
    # State programs can have input and output rings...reconnect them:
    # We're going to assume the connections are all valid since they got
    # saved:
    #
    set statePrograms [$::cs listObjects .c state-program]
    set rings         [$::cs listObjects .c ring]
    connectStateProgramsToRings $statePrograms $rings
    
    #  Hook data sources to their input rings.
    
    set dataSources [$::cs listObjects .c datasource]
    connectDsToInRings $dataSources $rings
    
    
}

##
# restoreState
#   Handler for the File->Restore... menu entry.
#   - Prompts for a file.
#   - Destroys current objects.
#   - Reads in objects from database.
#
proc restoreState {} {
    set file [tk_getOpenFile \
        -parent . -title {Select database} -defaultextension .db    \
        -filetypes [list [list {Database Files} .db]               \
                         [list {All files} * ] ]                    \
    ]
    if {$file eq ""} return;                # Cancelled.
    
    $::cs clearCanvas .c;                        # Get rid of all database objects.
    
    #  Now open the database file and restore all the stuff it has that affects
    #  us:
    
    set uri file://[file normalize $file]
    
    set services [::Serialize::deserializeServices $uri]
    installObjects $services
    
    set statePrograms [::Serialize::deserializeStatePrograms  $uri]
    installObjects $statePrograms
    
    set rings [::Serialize::deserializeRings $uri]
    installObjects $rings
    
    set evbs [::Serialize::deserializeEventBuilders $uri]
    installObjects $evbs
    
    set ds   [::Serialize::deserializeDataSources $uri]
    installObjects $ds
    
    restoreConnections $uri
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
.m add cascade -label View -menu .m.view
.m add cascade -label Help -menu .m.help


menu .m.file -tearoff 0
.m.file add command -label Save -command saveState
.m.file add command -label Restore... -command restoreState
.m.file add separator
.m.file add command -label Exit... -command exitProgram

menu .m.view -tearoff 0
.m.view add command -label Validation... -command validateConfiguration

menu .m.help -tearoff 0
.m.help add command -label About... -command showAbout

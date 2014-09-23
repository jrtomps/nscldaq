#!/usr/bin/env tclsh

package require cmdline
package require InstallRoot

# -----------------------------------------------------------------------------

set options [list  \
  [list inputring.arg "OfflineEVBIn"   "name of ring buffer event builder receives input from"] \
  [list outputring.arg "OfflineEVBOut" "name of ring buffer event builder outputs to"] \
  [list stagearea.arg  $::env(HOME)/offlinestagearea  "stagearea directory to use for output"] \
]

set usage " ?option value? ... :"
# if ? or help is determined as the parameter, then the getoptions returns an
# error consisting of the help...we catch it, print the help info, and then exit
if {[catch {array set params [::cmdline::getoptions argv $options $usage]} info]} {
  puts $info
  return
}

set inring [lindex [array get params inputring] 1]
set outring [lindex [array get params outputring] 1]
set stagearea [lindex [array get params stagearea] 1]

lappend auto_path [file join [::InstallRoot::Where] TclLibs] 


# -----------------------------------------------------------------------------
# Now the actual launcher code begins.
#
# Import all of the needed packages
#
package require OfflineOrdererUI
package require OfflineEVBGlobalConfigUI
package require FrameSequencer 


# prevent the tearoff of menus because those add more complexity to the UI than 
# is needed
option add *tearOff 0

# Set up the globals so that we know what they are for sure
set Globals::menu .m
set Globals::sequencer .seq

# create the menu
menu $Globals::menu
. configure -menu $Globals::menu


# rename our window and prevent resizing
wm title . "NSCLDAQ Offline Event Builder"
wm resizable . false false


# create the sequencer that mainly handles the switch from 
# global configuration and the job builder
set seq $Globals::sequencer
FrameSequencer $seq

# create the main frames that will be interacted with by the user
set GlobalConfig::win .glblConfig
set GlobalConfig::theInstance [GlobalConfigUIPresenter %AUTO% \
                                                   -widgetname .globalConfig]

set gc [::GlobalConfig::getInstance]
$gc setInputRing $inring
$gc setOutputRing $outring
$gc setStagearea $stagearea

set orderer [OfflineOrdererUIPresenter %AUTO% -widgetname .view]


# add these frames to the sequencer and then set it to the main configuration
# window
$seq add main   .view [list $Globals::menu add command -label "Config" \
                        -command { $seq select config ; $::Globals::menu delete 0 1} ]
$seq add config .globalConfig
$seq select main

# grid that beast
grid $seq -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1



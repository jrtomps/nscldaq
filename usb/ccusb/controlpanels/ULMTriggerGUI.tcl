#!/usr/bin/env tclsh

package require InstallRoot
package require cmdline
package require Itcl 

set options {
  {-module.arg ""         "name of module registered to slow-controls server \[MANDATORY\]"}
  {-slot.arg   -1         "slot ULM occupies \[MANDATORY\]" }
  {-host.arg "localhost"  "host running VMUSBReadout slow-controls server" }
  {-port.arg  27000       "port the slow-controls server is listening on" }
  {-ring.arg $::tcl_platform(user) "name of ring VMUSBReadout is filling" }
  {-configfile.arg "ulmtrigger.tcl"  "name of file to write GUI state to" }
}

set usage " --module value ?option value? :"
array set params [::cmdline::getoptions argv $options]

if {[lindex [array get params -module] 1] eq ""} {
  puts [::cmdline::usage $options $usage]
  puts "User must pass a valid value for the --module option"
  exit
}

if {[lindex [array get params -slot] 1] == -1} {
  puts [::cmdline::usage $options $usage]
  puts "User must pass a valid value for the --slot option"
  exit
}

set module [lindex [array get params -module] 1]
set slot [lindex [array get params -slot] 1]
set host [lindex [array get params -host] 1]
set port [lindex [array get params -port] 1]
set ring [lindex [array get params -ring] 1]
set path [lindex [array get params -configfile] 1]

# Ensure that we have the ULMTriggerGUI package in the lib
lappend auto_path [file join [::InstallRoot::Where] TclLibs] 
lappend auto_path [pwd]
package require ULMTriggerGUI

#source ULMTriggerGUIPanel.tcl

package require Tk

ATrigger2367 mygui 0 1 $host $port $module $slot 

if {$path ne ""} {
  mygui SetupGUI . $path
} else {
  tk_messageBox -icon error -message "User must supply a valid ---configfile argument"
}
# Configure some window manager details 
wm protocol . WM_DELETE_WINDOW { mygui OnExit ; destroy . } 
wm resizable . false false

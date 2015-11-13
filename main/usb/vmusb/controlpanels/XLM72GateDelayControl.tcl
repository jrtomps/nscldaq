#!/usr/bin/env tclsh

#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Team 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @author Jeromy Tompkins



# This file serves as a launcher for the XLM72GateDelayControl application.
# It merely processes the command line arguments and sets up the AGD16XLM72Panel
# object that will run during the session.
#

# Set up the package so that no body will
set here [file dirname [file normalize [info script]]]
lappend auto_path [file join $here .. TclLibs]
lappend auto_path [file join $here .. .. lib]

package require cmdline

set options {
  {module.arg ""         "name of module registered to slow-controls server \[MANDATORY\]"}
  {host.arg "localhost"  "host running VMUSBReadout slow-controls server" }
  {port.arg  27000       "port the slow-controls server is listening on" }
  {ring.arg "" "name of ring VMUSBReadout is filling" }
  {configfile.arg "" "name of configuration file to read from to initialize the GUI" }
}

set usage "-module value ?option value? :"
if {[catch {array set params [::cmdline::getoptions argv $options]} result]} {
  puts "Failed to parse command line arguments!"
}

if {[lindex [array get params module] 1] eq ""} {
  puts [::cmdline::usage $options $usage]
  puts "User must pass a valid value for the -module option"
  exit
}

set module [lindex [array get params module] 1]
set host [lindex [array get params host] 1]
set port [lindex [array get params port] 1]
set ring [lindex [array get params ring] 1]
set configFile [lindex [array get params configfile] 1]

if {$ring eq {}} {
  set ring tcp://localhost/$::tcl_platform(user)
}


package require Tk
package require gd16xlm72panel

AGD16XLM72Panel dev .gui $module $host $port $ring

if {([string length $configFile]>0)} {
  if {[file exists $configFile]} {
    dev SetConfigFileName $configFile
    dev ReadFileGD16
  } else {
    set errmsg "User specified configuration --configfile to be $configFile, but that does not exist"
    tk_messageBox -icon error -message $errmsg
    exit 0
  }
} else {
  puts "user did not set the configuration file"
}

grid .gui  -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1


# This is the default value of the signature that we know how to deal with.
# If the user wants to use something different, then they need to specify it.
#
set gd16(configuration) 0xdaba0006

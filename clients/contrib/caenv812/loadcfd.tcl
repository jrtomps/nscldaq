#!/bin/bash
#
#   This is a tclsh script to load a cfd given a configuration file
#   and a settings file.
#   The script can be run as a unix command.
#
#
#  Usage:
#    loadcfd config  settings
#
#   The config file is expected to set:
#      Crate       - VME Crate number for the module.
#      ModuleBase  - Module base address.
#
#
#   The settings file is expected to set:
#
#     Thresholds(0:15)          Threshold values.
#     WidthLow, WidthHigh       Widths for channels 0-7, 8-15
#     DeadTimeLow, DeadTimeHigh Channel deadtimes for 0-7, 8-15
#     Majority                  Majority logic level.
#     ChannelOn(0:15)           Channel enables (0 - channel disabled, 1 on).
#
#
#  Requires:
#     DISTRIBROOT environment variable to point to the SEE software 
#     distribution root.
#
# start tclsh \
exec tclsh $0 $@


set root $env(DISTRIBROOT)
set cfddir $root/controls/cfd          ;# The CFD support package is here.

lappend auto_path $cfddir
package require CFD812

#
#   Print out how to use this program:
#
#
proc Usage {} {
    puts "Usage:"
    puts "   loadcfd.tcl  configfile settingsfile"
    puts "Where:"
    puts "    configfile   - defines the location of the module."
    puts "    settingsfile - describes how to set up the module."
    
}

if {[llength $argv] != 2} {
    Usage
    exit
}


puts "$argv0 $argv"

set config   [lindex $argv 0]
set settings [lindex $argv 1]


source $config
source $settings

#
#   Create the module:
#
set module [CFD812::Map $ModuleBase "cfd" $Crate]

#
#  Disable the module while setting it up:
#
CFD812::SetMask $module 0

#
#   Set the thresholds
#

for {set i 0} {$i < 16} {incr i } {
   CFD812::SetThreshold $module $i $Thresholds($i)
}

#
#   Set the widths by bank:
#
CFD812::SetWidth $module 0 $WidthLow
CFD812::SetWidth $module 1 $WidthHigh

#
#  Set the deadtimes by bank:
#
CFD812::SetDeadtime $module 0 $DeadTimeLow
CFD812::SetDeadtime $module 1 $DeadTimeHigh
#
#  Set the multiplicity logic:
#
CFD812::SetMultiplicityThreshold $module $Majority

#
#  Compute the channel enable mask and set it.
#
set mask 0
for {set i 0} {$i < 16} {incr i} {
    if {$ChannelOn($i)} {
	set mask [expr $mask | (1 << $i)]
    }
}
CFD812::SetMask $module $mask

exit 0
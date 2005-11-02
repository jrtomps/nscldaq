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
#     mask_arr(0:15)           Channel enables (0 - channel disabled, 1 on).
#
#
#  Requires:
#     DISTRIBROOT environment variable to point to the SEE software
#     distribution root.
#
# start tclsh \
exec tclsh $0 $@


set me [info script]

# Canonicalize the directory one above me so that we can search
# for packages in the tree rooted in ..

set libdirs [file join [file dirname $me] ..]
set dir [pwd]
cd $libdirs
set libdirs [pwd]
cd $dir

#  If $libdirs is not part of the path yet, then
#  prepend it to the path so that it overrides any bad obsolete dirs.
#

if {[lsearch -exact $auto_path $libdirs] == -1} {
    set auto_path [linsert $auto_path 0 $libdirs]
}

package require CFD812 2.0

#
#   Print out how to use this program:
#
#
proc Usage {} {
    puts "Usage:"
    puts "   loadcfd.tcl  configfile ?settingsfile?"
    puts "Where:"
    puts "    configfile   - defines the location of the module."
    puts "    settingsfile - describes how to set up the module."

}

if {([llength $argv] > 2) || ([llength $argv] < 1)} {
    Usage
    exit
}


puts "$argv0 $argv"

set config   [lindex $argv 0]
source $config


if {[llength $argv] == 2} {
    set settings [lindex $argv 1]
    source $settings
}

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
    if {[array names Threshold $i] != ""} {
	CFD812::SetThreshold $module $i $Thresholds($i)
    }
}

#
#   Set the widths by bank:
#
if {[info var WidthLow] != ""} {
    CFD812::SetWidth $module 0 $WidthLow
}
if {[info var WidthHigh] != ""} {
    CFD812::SetWidth $module 1 $WidthHigh
}

#
#  Set the deadtimes by bank:
#
if {[info var DeadTimeLow] != ""} {
    CFD812::SetDeadtime $module 0 $DeadTimeLow
}
if {[info var DeadTimeHigh] != ""} {
    CFD812::SetDeadtime $module 1 $DeadTimeHigh
}
#
#  Set the multiplicity logic:
#
if {[info var Majority] != ""} {
    CFD812::SetMultiplicityThreshold $module $Majority
}

#
#  Compute the channel enable mask and set it.
#
set mask 0
for {set i 0} {$i < 16} {incr i} {
    if {[array names mask_arr $i] != ""} {
	if {$mask_arr($i)} {
	    set mask [expr $mask | (1 << $i)]
	}
    }
}
CFD812::SetMask $module $mask

exit 0

#!/bin/bash
#
#  Simple script to load a shaper with the set of values in a settings file.
#  Note that in SEE, only the first 8 channels of the shaper are used.
#
#  Usage:
#     loadshaper.tcl   config  settings
#
#      config is a file that contains the configuration of the shaper.
#      module.  It is expected to set:
#      controller               - Name of the CAENnet controller.
#      nodeid                   - CAENnet node of the shaper.
#
#      Note: The controller name is of the form caennet_baseaddress.
#      where baseaddress is the base address in VME space of the caennet
#      controller.
#
#      settings is a file that contains the values to program into the shaper.
#      It is assumed to set:
#
#       offset                   - Module common offset
#       outconfig(0:7)           - Selects the output configuration.
#       outpolarity(0:7)         - Selects the channel input polarity.
#       shapetime(0:7)           - Selects the channel shaping times.
#       coarsegain(0:7)          - Selects the channel coarse gains.
#       finegain(0:7)            - Selects the channel fine gains.
#       pole0(0:7)               - per channel pole0 adjust.
#      Regardless of the request, the multiplexor is disabled.
#      The module is programmed as suggested onpage 12 of the N586B manual.
#
# Start tclsh \
exec tclsh $0 $@ 

#
#   Locate out package directory and require the shaper support package from 
#   it
#

set root $env(DISTRIBROOT)
set shaperdir $root/controls/shaper

source $shaperdir/n568b.tcl

package require caennet
package require n568b

#
#   list the program usage.
#
proc Usage {} {
    puts "Usage: "
    puts "   loadshaper.tcl config settings"
    puts "Where:"
    puts "   config    - is the shaper module configuration file."
    puts "   settings  - is the shaper module settings file."

}


#
#   Source in the configuration files:
#

if {[llength $argv] != 2} {
    Usage
    exit
}

set config   [lindex $argv 0]
set settings [lindex $argv 1]

source $config
source $settings

#
#  Parse the controller to produce the caennet  base address (crate 0)
#  create the controller and reset the caennet.
#

set ctllist [split $controller _]
set base    [lindex $ctllist 1]

set module [caennet::create $base]
caennet::reset $module


#
#  Set the output configuration on all channels:
#
puts -nonewline "Output configuration..."
for {set i 0} {$i < 8} {incr i} {
    n568b::SetOutputConfiguration $module $nodeid $i $outconfig($i)
}
puts ""

#
#   Set the output polarity on all channels:
#
puts -nonewline "Output Polarity..."
for {set i 0} {$i < 8} {incr i} {
    n568b::SetPolarity $module $nodeid $i $outpolarity($i)
}
puts ""
#
#  Set the shaping time on all channels:
#
puts -nonewline "Shaping times..."
for {set i 0} {$i < 8} {incr i} {
    n568b::SetShapingTime $module $nodeid $i $shapetime($i)
}
puts ""
#
#  Set the coarse gain on all channels:
#
puts -nonewline "Coarse Gain..."
for {set i 0} {$i < 8} {incr i} {
    n568b::SetCoarseGain $module $nodeid $i $coarsegain($i)
}
puts ""
#
#  Set the fine gain on all channels:
#
puts -nonewline "Fine gain..."
for {set i 0} {$i < 8} {incr i} {
    n568b::SetFineGain $module $nodeid $i $finegain($i)
}
puts ""
#
#   Set the pole 0 adjust on all channels:
#
puts -nonewline "Pole 0 adjust..."
for {set i 0} {$i < 8} {incr i} {
    n568b::SetPole0 $module $nodeid $i $pole0($i)
}
puts ""
#
#   Set the common offset
#
n568b::SetOffset $module $nodeid $offset
puts "Common offset set"

#
#  Disable the mux:
#
n568b::DisableMuxOut $module $nodeid
puts "Mux disabled"
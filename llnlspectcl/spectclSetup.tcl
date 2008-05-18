#
#   Setup SpecTcl's unpacking.
#    This script sets up SpecTcl's auto unpack system for data
#    from the VM-USB.
#    The script processes the user's daqconfig file
#    using the configFile script.
#
#  That script is going to create the following global arrays that we will use:
#    adcConfiguration - indexed by digitizer name, this contains the 
#                       virtual slot numbers of digitizers that have them and -1 for those that
#                       don't.
#    readoutDeviceType- indexed by digitizer name, this contains the device type (a small integer)
#    stackOrder       - indexed by stack name, this contains the order in which modules
#                       are read in a stack.
#    stackNumber      - indexed by stack name, this contains the number of that stack.
#
#  The users's configuration file is supposed to have created the array:
#    adcChannels      - The adcChannels array is supposed to be indexed by digitizer
#                       name and provide SpecTcl parameter names for each channel of the
#                       digitizer.
#                       
# As a bonus we set up raw parameter spectra for each parameter that is defined.
#
# 


set here [file dirname [info script]]
source [file join $here configFile.tcl]


configClear
configRead [file join $here daqconfig.tcl]

set channelCount($typeCAEN)   4096
set channelCount($typeHYTEC)  8192



#-----------------------------------------------------------------------------
# Creates a 1-d spectrum.
#

proc makeSpectrum {paramname channels} {
    set low 0
    set high [expr $channels-1]

    spectrum $paramname 1 $paramname  [list [list $low $high $channels]]
}

#----------------------------------------------------------------------------
# Build the channel maps, spectcl parameters and raw spectra from 
# the adcConfigurtion, readoutDeviceType and adcChannels information.
# This will all be driven by the adcCahnnels array.
#
# Parameters:
#   param  - the number of the first parameter.

proc buildChannelMaps param {
    foreach module [array names ::adcChannels] {
	set vsn        $::adcConfiguration($module)
	set type       $::readoutDeviceType($module)
	set resolution $::channelCount($type)
	set channels   $::adcChannels($module)

	# Make the parameters and spectra:

	foreach parameter $channels {
	    parameter $parameter $param
	    incr param
	    makeSpectrum $parameter $resolution
	}
	#  Give SpecTcl the parameter map for the module:

	paramMap $module $type $vsn $channels
    }
}


#--------------------------------------------------------------------------
# Build the stack order maps.  These define, for each stack,
# the order in which the modules read by that stack appear.
#
#
proc buildStackMaps {} {
    #
    # Drive off the stack number since stacks that can't be assigned a number
    # can't actually be decoded.
    #
    foreach stack [array names ::stackNumber] {
	stackMap $::stackNumber($stack) $::stackOrder($stack)
    }
}

#--------------------------------------------------------------------------
#
#  Setup SpecTcl
#

buildChannelMaps 20

buildStackMaps


sbind -all



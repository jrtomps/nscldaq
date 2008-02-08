#
#  This tcl file is used by SpecTcl to establish 
#  parameter maps for the digitizers that have been
#  declared in the system.
#

# We are going to operate as follows:
#  1. Interpret the daqconfig.tcl file really only looking for
#     the -modules lists on the events stack.
#  2. Expect arrays of the form parameters(digitizer) that have
#     the parameter names for the specific digitizer.
#  3. For each digitizer in the list, create/execute a parammap
#     Create spectra (4k raw channel spectra).
#


set daqconfig [file join ~ config daqconfig.tcl]

#
#  Ignore the ph7xxx module definitions/configurations.
#

proc ph7xxx args {
}

#
#  There could be two stacks.. a scaler stack and an
#  events stack...though there's currently no support for scalers.
#  This means that we must build up the configuration for each stack.
#  This is done in the stackConfig global array.
#
#  Note that configuration argument pairs all start
#  at the 3'd (from 0) argument
#
proc stack args {
    global stackConfig
    set name   [lindex $args 1]
    set config [lrange $args 2 end]
    foreach {key value} $config {
	append stackConfig($name) " " $key " " [list $value]
    }
}

#
#   Procees the stackConfig array to get the module list.
#   for the event stack.
#
#  The strategy will be to throw the configuration of each module
#  into a local array which we then look at as follows:
#   If the -type is event we care about it.
#   look at the -modules value and return it.
#  Note that array set is cumulative. Hence the unset.
#
proc getModuleList {} {
    global stackConfig
    
    foreach stack [array names stackConfig] {
	array set config $stackConfig($stack)

	if {$config(-type) eq "event"} {
	    return $config(-modules)
	}
	unset config;             # For next time.
    }
    error "There is no event stack!!"
}
#
#   Create the parameter maps and spectra
#   for the set of modules specified.
#   for each module we expect parameter(modulename) that is a list
#   of the parameters for that module.
#
proc createMapAndSpectra modules {
    global parameters
    set moduleNumber 0
    foreach module $modules {
	if {[array names parameters $module] ne ""} {
	    parammap -add $moduleNumber $parameters($module)
	    foreach parameter $parameters($module) {
		spectrum $parameter 1 $parameter {{0 4095 4096}}
	    }
	}
	incr moduleNumber
    }
}


source $daqconfig

set modules [getModuleList]
createMapAndSpectra  $modules


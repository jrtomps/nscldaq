#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  This file provides software that will decode a configuration file.
#  the only thing we are interested in are the adc lines.  From them
#  we are interested in the names, types, order and geographical slots.
#  The following global arrays will be created arrays are indexed by device
#  name:
#
#    adcConfiguration - Contains virtual slot numbers for devices that have them
#                       -1 for those that don't.
#    readoutDeviceType - Contains the device type code for each device.
#                        See below for device types.
#    stackOrder        - Indexed by stack name.  The order of the modules
#                       in the stack.  If a chain is in the list,
#                       its name is replaced by the order of the modules
#                       in the chain.
#    stackNumber       - Indexed by stack name. Contains the stack number.
#                       The stack number is:
#                       0  - Un numbered nim1 triggered stack.
#                       1  - Un numbered scaler triggered stack.
#                       n  -  value of -stack  config parameter.
#
#  Global variables that are used but not important to the outside world:
#      chainOrder      - For CAENchain, the order in which the
#                        devices appear, indexed by chain.
#
#

# Define the device types so the decoder can be selected:

set typeCAEN  0;			# CAEN V775,785,792,862
set typeHYTEC 1;			# Hytec NADC 2530.
set typeMADC32 2;			# Mesytec MADC 32.


#  We create as well spectra for each single parameter, and corresponding
#  pairs of n's.

package provide configFile 1.0

# we will be maintaining a global array
# array will be indexed by adc name.  Each array will contain the
# geo address assigned to the adc.


#-----------------------------------------------------------------
#
#   clear the configuration.
#   The catch is required in case the config does not (yet) exist.
#
proc configClear {} {
    global adcConfiguration
    catch {unset adcConfiguration}
}
#------------------------------------------------------------------
#  Read a configuration file.  This is just a source.
#
proc configRead filename {
    uplevel #0 source $filename
}


#-------------------------------------------------------------------
# adcCreate
#   Creates a new adc; Creates and adc with the default geo value (0).
#   presumably, the  adcConfig will override this value later.
#
proc adcCreate tail {
    global adcConfiguration

    set name [lindex $tail 0]
    set adcConfiguration($name) 0
    set ::readoutDeviceType($name) $::typeCAEN
}
#-------------------------------------------------------------------
# adcConfig
#   Configures an adc. For now we are only interested in the
#   -geo switch which will have our virtual slot number.
#
proc adcConfig tail {
    global adcConfiguration

    set name [lindex $tail 0]
    set config [lrange $tail 1 end]
    foreach {key value} $config {
	if {$key eq "-geo"} {
	    set ::adcConfiguration($name) $value
	}
    }
}
#-------------------------------------------------------------------
# adc - processes the adc command dispatches to the create/config
#       commands.  All other subcommands are no-ops.
#
proc adc args {
    set subcommand [lindex $args 0]
    set tail       [lrange $args 1 end]
    
    if {$subcommand eq "create"} {
	adcCreate $tail
    }
    if {$subcommand eq "config"} {
	adcConfig $tail
    }
}

#----------------------------------------------------------------
#  The hytec command processes creation and configuration commands
#  for the  hytec N2530 ADC. 
#
proc hytec args {
    set subcommand [lindex $args 0]
    set name       [lindex $args 1]
    
    set ::readoutDeviceType($name) $::typeHYTEC
    set ::adcConfiguration($name)  -1;	# There is no geo associated with this adc.
}

#---------------------------------------------------------------
# The madc command processes the creation and configuration of
# Mesytec 32 channel ADC modulees.
#
proc madc args {
    set subcommand [lindex $args 0]
    set name       [lindex $args 1]

    set ::readoutDeviceType($name) $::typeMADC32

    # The config or create subcommand have the 
    # -id config which sets the 'vsn' for this module.

    if {($subcommand eq "create") || ($subcommand eq "config")} {
	set ididx [lsearch -exact $args "-id"]
	if {$ididx != -1} {
	    incr ididx
	    set ::adcConfiguration($name) [lindex $args $ididx]
	}
    }
}
#---------------------------------------------------------------
#  We need to use this command to fill in the chainOrder array of 
#  the order of modules in the chain.  This allows the stack command
#  to accurately figure out the module order in the presence of
#  chains that aggregate several V785 readout 'devices'.
#
#  - Extract the chain name.
#  - If the args contains -modules extract the next list item
#    and use it as the value for the chain's chainOrder entry.
#
proc caenchain args {
    set name     [lindex $args 1]
    set modIndex [lsearch -exact $args "-modules"]
    if {$modIndex != -1} {
	incr modIndex
	set ::chainOrder($name) [lindex $args $modIndex]
    }
}
#---------------------------------------------------------------
#
#  For each of the stacks, the stack command must create a
#  stackNumber and stackOrder entry.
#
proc stack args {
    set stackname [lindex $args 1]
    
    # Hunt for the modules config and fill in stackOrder if so.
    # Chains are dealt with here too.
    
    set moduleIndex [lsearch -exact $args "-modules"]

    if {$moduleIndex != -1} {
	incr moduleIndex
	set moduleOrder [lindex $args $moduleIndex]
	foreach name $moduleOrder {
	    if {[array names ::chainOrder $name] ne ""} {
		foreach item $::chainOrder($name) {
		    lappend ::stackOrder($stackname) $item
		}
	    } else {
		lappend ::stackOrder($stackname) $name
	    }
	}
    }

   # Figure out the stack number. Simplifying assumption is that
    # users of the nim1, and scaler trigger will not specify a -stack
    # entry or if they do, it will be the correct stack number.

    set trigIndex [lsearch -exact $args "-trigger"]
    if {$trigIndex != -1} {
	incr trigIndex
	set trigger [lindex $args $trigIndex]
	if {$trigger eq "nim1"} {
	    set ::stackNumber($stackname) 0
	} elseif {$trigger eq "scaler"} {
	    set ::stackNumber($stackname) 1
	}
    }  
    set stackIndex [lsearch -exact $args "-stack"]
    if {$stackIndex != -1} {
	incr stackIndex
	set ::stackNumber($stackname) [lindex $args $stackIndex]
    }    
}


# 
# The configuration file can have serveral commands we don't need to
# deal with in SpecTcl...
# these are defined as procs that do nothing:
#
proc sis3820 args {
}
proc v830 args {
}




proc marker args {
}
proc sis3804 args {
}
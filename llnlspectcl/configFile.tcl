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
#  we are interested in the names, and geographical slots.

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
#------------------------------------------------------------------
#  Read a configuration file.  This is just a source.
#
}
proc configRead filename {
    source $filename
}

#---------------------------------------------------------------------
# scaler - processes the scaler command.  This is a no-op.
#
proc scaler args {
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
	    set adcConfiguration($name) $value
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
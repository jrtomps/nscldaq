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

# This package captures and encapsulates the
# configuration information of the data acquisition subsystem.
# This configuration data controls how Readout is started, what
# it is fed for a first run number, and how data are recorded.
# Configuration parameters are:
#
# Parameter Name   Env Name      Meaning
# EventLogger      EVENTLOGGER   Path to the event log program.
#                                Allows you to select between the default
#                                which produces ring buffer event files and
#                                the compatibility mode event logger.
#

package provide DAQParameters 1.0
package require Configuration
package require InstallRoot


namespace eval DAQParameters {
}
# DAQParameters::setDefaults
#  Initialize the default values for each of the parameters.
#  The following parameters have defaults:
#
#    BufferSize   - 4096 (words)
#
proc DAQParameters::setDefaults {} {
    
    Configuration::Set EventLogger [file join [InstallRoot::Where] bin eventlog]
    Configuration::Set EventLoggerRing "tcp://localhost/$::tcl_platform(user)"
}
# DAQParameters::environmentOverrides
#   Overrides the defaults with information from the environment
#   See the file comment header for information about which
#   items support env overrides.
#
proc DAQParameters::environmentOverrides {} {
    global env


    Configuration::readEnvironment EventLogger EVENTLOGGER
    Configuration::readEnvironment EventLoggerRing EVENTLOGGER_RING
}

#
#  Return the event logger program name:
#
#
proc DAQParameters::getEventLogger {}  {
    return [Configuration::get EventLogger]
}
##
#  Return the name of the ring from which data are logged:
#
proc DAQParameters::getEventLoggerRing {} {
    return [Configuration::get EventLoggerRing]
}

#  Initialize on load:

DAQParameters::setDefaults
DAQParameters::environmentOverrides

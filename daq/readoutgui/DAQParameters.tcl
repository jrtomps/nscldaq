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
    Configuration::Set EventLogUseNsrcsFlag      1
    Configuration::Set EventLogAdditionalSources 0
    Configuration::Set EventLogUseGUIRunNumber   0
}
# DAQParameters::environmentOverrides
#   Overrides the defaults with information from the environment
#   See the file comment header for information about which
#   items support env overrides.
#
proc DAQParameters::environmentOverrides {} {
    global env


    Configuration::readEnvironment EventLogger               EVENTLOGGER
    Configuration::readEnvironment EventLoggerRing           EVENTLOGGER_RING
    Configuration::readEnvironment EventLogUseNsrcsFlag      EVENTLOGGER_NSRCSFLAGS_SUPPORTED
    Configuration::readEnvironment EventLogAdditionalSources EVENTLOGGER_UNCONTROLLED_SOURCE_COUNT
    Configuration::readEnvironment EventLogUseGUIRunNumber   EVENTLOGGER_USE_GUI_RUNNUM
    
}

##
# DAQParameters::getEventLogger
#
# @return - the event logger program name:
#
#
proc DAQParameters::getEventLogger {}  {
    return [Configuration::get EventLogger]
}
##
# DAQParameters::getEventLoggerRing
#
#  @return Name of the ring from which data are logged:
#
proc DAQParameters::getEventLoggerRing {} {
    return [Configuration::get EventLoggerRing]
}
##
# DAQParameters::getUseNsrcsFlag
#
# @return boolean - true if the --number-of-sources flag should be
#                   supplied to the event logger command.  The value used will
#                   be the number of sources managed by the data source manager
#                   added to the uncontrolled source count (see
#                   DAQParameters::getAdditionalSourceCount below).
#
proc DAQParameters::getUseNsrcsFlag {} {
    return [Configuration::get EventLogUseNsrcsFlag]
}
##
# DAQParameters::getAdditionalSourceCount
#
# @return integer Number of uncontrolled sources.  Note there's a pathology
#         that allows this to be negative.  Suppose a data source can be
#         controlled but does not produce state transition events
#         (e.g. we're event building with GRETINA) in that case we can't let
#         the event logger count that source.
#
proc DAQParameters::getAdditionalSourceCount {} {
    return [Configuration::get EventLogAdditionalSources]
}
##
# DAQParameters::getRunNumberOverrideFlag
#
# @return bool - If true, the --run switch should be supplied to the
#                event logger overriding (or supplying) run number information
#                that presumably no event sources produce.   Note that
#                this just affects the name of the event file, not its contents.
#
proc  DAQParameters::getRunNumberOverrideFlag {} {
    return [Configuration::get EventLogUseGUIRunNumber]
}

#  Initialize on load:

DAQParameters::setDefaults
DAQParameters::environmentOverrides

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file readoutGuiProvider.tcl
# @brief layer on top of an s800Provider that gives remote control of a ReadoutGUI
# @author Ron Fox <fox@nscl.msu.edu>

package provide RemoteGUI_Provider 1.0
package require S800_Provider
package require ui
package require ReadoutGuiClient


##
# This version only supports a single server
# TODO:  support multiple servers/sources.
#

namespace eval ::RemoteGUI {
    variable host
    variable user
    variable sourceid
    variable outputMonitor
}

##
# parameters
#     Returns the set of parameters that are required to create the source
#    - host  - the host in which the readoutGUI is running
#    - user  - Username under which the readoutGUI is running
#
proc ::RemoteGUI::parameters {} {
    return [dict create host [list {Host Name}] user [list {User name}]]
}
##
# start
#    Start the data source.  This means
#    - Locating the run control port
#    - Locating the monitor port.
#    - Settint up to monitor output
#    - delegating the start to the s800::start.
#
# @param params - Parameterization of the source.
#
proc ::RemoteGUI::start params {
    variable host
    variable user
    variable sourceid
    variable outputMonitor

    set host [dict get $params host]
    set user [dict get $params user]

    set sourceid [dict get $params sourceid]
    
    # Locate the s800 data source port, and attempt to start the data source
    # that will complain if there's already one.
                  
    set port [readoutGUIControlPort $host $user]
    if {$port eq ""} {
        error "Unable to find a readout GUI in $host run by $user"
    }
    S800::start [dict create port $port host $host sourceid $sourceid]
    
    # Attempt to locate the output window and connect to it as well:
    
    set outputMonitor [ReadoutGUIOutputClient %AUTO% -host $host -user $user \
                        -outputcmd ::RemoteGUI::_handleOutput \
                        -closecmd ::RemoteGUI::_outserverClosed]
    $outputMonitor connect
}
##
# check - See if we are still alive:
#
# @param id - the source id.
#
proc ::RemoteGUI::check id {
    return [S800::check $id]
}
##
# stop - stop the source by stopping the s800 and destroying the output
#        monitor.
# @param id - source id.
#
proc ::RemoteGUI::stop id {
    variable outputMonitor
    
    S800::stop $id
    $outputMonitor destroy
}
##
# begin - start a run.
# @param id - source id
# @param run - run numer
# @param title
#
proc ::RemoteGUI::begin {id run title} {
    ::S800::begin $id $run $title

}
##
# end
#   End the run
# @param id - the source id
#
proc ::RemoteGUI::end id {
    ::S800::end $id
}
##
# capbilities - the s800 capabilities:
#
proc ::RemoteGUI::capabilities {} {
    return [::S800::capabilities]
}

#------------------------------------------------------------------------------
#
#  Private methods:
#

##
# _handleOutput
#
#  Called to handle output from the remote Readout GUI ..just output it to our
#  display
#
# @param output - output from the remote
#
proc ::RemoteGUI::_handleOutput output {
    set ow [::Output::getInstance]
    $ow puts $output
}
##
# _outserverClosed
#
#  Called when the output server closes...probably the ReadoutGUI has blown
#  away.  Just destroy the connection object and complain to the ow.
#
proc ::RemoteGUI::_outserverClosed {} {
    variable outputMonitor
    $outputMonitor destroy
    
    set ow [::Output::getInstance]
    $ow log error "Lost connection to remote GUI output server."
    $ow log error "Probably the control server connection already was lost or soon will be"
}
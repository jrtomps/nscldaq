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
# @file sshProvider.tcl
# @brief Provide a data source on the end of an ssh pipeline.
# @author Ron Fox
# @note There can be only one instance at this time.  Since we may  need to modify
#       that later we're going to encapsulate state access in helper procs that
#       are parameterized with the source id.
#

package provide S800_Provider 1.0
package require s800
package require ReadoutGUIPanel

# Establish the namespace in which the methods and state live and
# define the state variables.
#

namespace eval ::S800 {
    variable host     ""
    variable port     ""
    variable sourceid ""
    variable rctlObject ""
    
    #
    #  This variable contains the state of the connection and run:
    #  halted - there is no connection to an s800 readout.
    #  idle   - The s800 readout is connected but not taking data
    #  active - The S800 readout is connected and taking data.
    #
    variable state  halted;
    
}

#------------------------------------------------------------------------------
#  Public interface of the data source provider.

##
# parameters
#    Returns the set of parameters that are required to create the source.
#    For now these are:
#    - host  - The TCP/IP host that is running the control server.  IP or DNS ok.
#    - port  - The TCP/IP host on which the control server listens for input.
#
proc ::S800::parameters {} {
    return [dict create host [list {Host name}] port [list {Run control server port}]]
}

##
# start
#   Start a new data source:
#   * The state must be halted.
#   * Fill in the local state from the parameterization.  Note that this is
#     done via helpers to make it easier to go to a multi-control thing.
#     only the state accessed directly to check that we _are_ a single intance.
#
# @param params - Dict containing the parameterization of the source.
#
# @throw  - Source is already active.
#
proc ::S800::start params {
    set sid     [dict get $params sourceid]
    set host    [dict get $params host]
    set port    [dict get $params port]
    
    # >> This needs to be changed if this becomes multi server capable.
    
    if {$::S800::state ne "halted"} {
        error \
            "There is already an S800 data source connected and only one is supported."
    } else {
        set ::S800::sourceid $sid
    }
    #  Save
    
    ::S800::_setHost $sid $host
    ::S800::_setPort $sid $port
    set connection [s800rctl %AUTO% -host $host -port $port]
    ::S800::_setConnectionObject $sid $connection
    ::S800::_setState $sid idle
    
    $connection setSlave;    # Take over control.
    
}
##
# check
#   Check the status of the connection to the source.
#   The source's connection object is gotten and a status request is done.
#   that will fail if the connection has failed.
#
# @param id - Source id of the provider instance.
# @return boolean true if connected. false if not.
# @throw - it's an error to provide a source id that is not managed by us.
#
proc ::S800::check id {
    set rctl [::S800::_getConnectionObject $id]
    
    set bad [catch {$rctl getState} value]
    if {$bad} {
        ::S800::_failed $id
        return 0
    }  else {
        return 1
    }
}
##
# stop
#   Stop the data source.  In this case we don't stop the S800, but instead
#   ensure the run is ended and disconnect.
#
# @param id - Id of the source to close.
# @throw it is an error to provide a source that is not ours.
#
proc ::S800::stop id {
    
    # If still alive and necessary stop the run.
    
    if {([::S800::_getState $id] ne "halted") && [::S800::check $id]} {
        set rctl [::S800::_getConnectionObject $id]
        set status [$rctl getState]
        if {$status eq "active"} {
            $rctl end
        }
        # Regardless, _failed will run down the rest of this.
        
        ::S800::_failed $id;    # Will do all the right stuff.
    }
    # Already in halted state since check took care of that for us.
}
##
# begin
#   Attempt to start a run.
#
# @param id - Source id.
# @param run  - Run number desired.
# @param title - Title desired.
#
# @note - this business of slave/master mode is really crappy.  It's possible
#         somebody grabbed control at the control panel, started a run and
#         left it running... if that's the case we're going to try to re-assert
#         slave mode and stop that run before proceeding.
#
# @throw - it's an error to pass in a source id we don't control.
# @throw - it's an error if we've lost touch with the s800.
#
proc ::S800::begin {id run title} {
    ::S800::_errorIfDead $id
    
    # We're going to use the state from the s800 because maybe some bozo
    # grabbed it and shook it themselves:
    
    set rctl [::S800::_getConnectionObject $id]
    set state [$rctl getState]
    if {$state eq "active"} {
        $rctl setSlave
        $rctl end;           # End whatever nonesense is active now.
    }
    $rctl setRun $run
    $rctl setTitle $title
    $rctl begin
    ::S800::_setState $id active;   # Regarless we're taking data.
}
##
# @note S800 does not support pause and resume operations.

##
# end
#    Ends an active run.  See the note for begin about why this is not as
#    simple as we'd hope.
#
# @param id - Id of the source to end.
#
proc ::S800::end id {
    ::S800::_errorIfDead $id
    
    # Check the s800 state.. if the state is already not active,
    # we don't ask it to do anything but give us our slave control back.
    
    set rctl [::S800::_getConnectionObject $id]
    set state [$rctl getState]
    if {$state eq "active"} {
        $rctl end
    } else {
        $rctl setSlave
    }
    ::S800::_setState $id idle
}
##
# capabilities
#   Returns a dict describing the capabilities of the source:
#   - canpause - false
#   - runsHaevTitles - true
#   - runsHaveNumbers - true
#
# @return dict - as described above.
#
proc S800::capabilities {} {
    return [dict create \
        canPause        false   \
        runsHaveTitles  true    \
        runsHaveNumbers true    \
    ]
}
#------------------------------------------------------------------------------
#  Utility methods:
#

##
# _checkId
#    Error if the id does not match the one we're using
#
# @param id - The id to checke
# @throw if this does not match ::S800::sourceid
#
proc ::S800::_checkId id {
    if {$id != $::S800::sourceid} {
        error "$id is not the id of a source managed by the S800 provider."
    }
}
##
# _setHost
#   Set the host parameter for the specified data source
#
# @param id - The data source id.
# @param host - The host parameter value.
#
# @throw if the id is not one of ours.
#
proc ::S800::_setHost {id host} {
    ::S800::_checkId $id
    
    set ::S800::host $host
}
##
# _setPort
#
#  Set the port parameter of the data source
#
# @param id - Id of the data source.
# @param port - TCP/IP port on which the s800 is listening for connections.
# @throw If id is not managed by us.
#
proc ::S800::_setPort {id port} {
    ::S800::_checkId $id
    set ::S800::port $port
}
##
# _setConnectionObject
#
#  Sets the connection object that manages communication with a data source.
#  this is an object of type s800rctl.
#
# @param id - The id of the data source.
# @param objname - Name of the object.
#
# @throw If id is not managed by us.
#
proc ::S800::_setConnectionObject {id objname} {
    ::S800::_checkId $id
    set ::S800::rctlObject  $objname
}
##
# _setState
#
#  Set the state of a connection.
#
# @param id - Connection id.
# @param state - new state.
# @throw If id is not managed by us.
#
proc ::S800::_setState {id state} {
    ::S800::_checkId $id
    set ::S800::state $state
}
##
# _getState
#
#   Return the state of the source
#
# @param id - source id.
# @return the current state
#
proc ::S800::_getState id {
    ::S800::_checkId $id
    return $::S800::state
}
##
# _getConnectionObject
#
#  Returns the connection object associated with a source.  The connection object
#  is the s800rctl object that manages communication with the data source.
#
# @param id - source id.
# @return The connection object.
# @throw If id is not managed by us.
#
proc ::S800::_getConnectionObject {id} {
    ::S800::_checkId $id
    return $::S800::rctlObject
}
##
# _failed
#
#  Manages the 'failure' of communcations with the s800.  Note this also
#  coincidentally also properly handles normal close of the s800 command link.
#
# @param  id - the id of the data source.
#
proc ::S800::_failed id {
    ::S800::_checkId $id
    
    $::S800::rctlObject destroy
    set ::S800::rctlObject [list]
    set ::S800::state halted
}
##
# _errorIfDead
#  Check the state of the s800 and if it is dead, throw an error.
#
# @param id -Source id.
#
proc ::S800::_errorIfDead id {
    set state [::S800::check $id]
    if {!$state} {
        error "S800 Connection is dead for source $id."
    }
}

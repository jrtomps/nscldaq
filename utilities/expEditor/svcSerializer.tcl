#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file svcSerializer.tcl
# @brief Serialization for service objects.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide svcSerializer 1.0
package require daqservices

# Force the ::Serialize namespace into existence.

namespace eval ::Serialize {
    namespace export serializeServices
}

#
# @note - all procs whose names begin in _ should be considered private.  The
#         API is only those procs that don't start in _.

##
# Serialize::_destroyExistingServices
#
#   Destroy any services the database already has.  This is the simplest way
#   to deal with services that get deleted during editing -- start fresh.
#
# @param api - api command.
#
proc ::Serialize::_destroyExistingServices {api} {
    set svcs [$api listall]
    foreach [list svcname svcdata] $svcs {
        $api remove $svcname
    }
}
##
# ::Serialize::_saveService
#    Save a single service to the database.  This proc assumes there are no
#    existing services with the name of our service.  This will be the case if
#    *  _destroyExistingServices was called prior to us.
#    *  The caller ensured there are no duplicate names in the list of objects
#       to serialize.
# @param api - the api object command.
# @param obj - command of object to save.
#

proc ::Serialize::_saveService {api obj}  {
    set props [$obj getProperties]
    
    # We need the name, command, host, and x/y.
    
    set p [$props find name]
    set name [$p cget -value]
    
    set p [$props find host]
    set host [$p cget -value]
    
    set p [$props find path]
    set path [$p cget -value]
    
    set position [$obj getPosition]
    
    # Now we can create the object and position it:
    
    $api createprog $name $path $host
    $api setEditorPosition $name [lindex $position 0] [lindex $position 1]
}


##
# Serialize::serializeServices
#   Replace all services in an existing variable database with the set of
#   services known by the system.
#
# @param dbURI   - URI for database requests or file.
# @param objects - List of object commands for the services that will be serialized.
#
proc ::Serialize::serializeServices {dbURI objects} {
    
    # Create an api and initialize the db if needed:
    
    ::nscldaq::services _svcSerialize $dbURI
    _svcSerialize create;                 # Ensure the infrastructure exists.
    
    # destroy any existing services.
    
    ::Serialize::_destroyExistingServices _svcSerialize
    
    
    # Create the services we have.
    
    foreach obj $objects {
        ::Serialize::_saveService _svcSerialize $obj
    }
    
    # Destroy the  API object now that we are done.
    
    ::nscldaq::services -delete _svcSerialize
}

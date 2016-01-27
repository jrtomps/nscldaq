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
# @file evbSerializer.tcl
# @brief Serialize event builders to/from data base files.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide evbSerializer 1.0
package require vardbEventBuilder

##
# @note procs whose names (after namespace) begin with _ are considered private.

##
#  Ensure the ::Serialize namespace is defined:
#
namespace eval ::Serialize {
    
    namespace export serializeEventBuilders
}

##
# ::Serialize::_destroyEventBuilders
#    Destroy existing event buidlers and their data sources.
#
# @param api - command word for the api object.
#
proc ::Serialize::_destroyEventBuilders api {
    set eventBuilders [$api evbList]
    foreach evb $eventBuilders {
        set name [dict get $evb name]
        $api rmevb $name
    }
}
##
# ::Serialize::_saveEventBuilder
#
#   Save a single event builder to the database.
#
# @param api   - api command word.
# @param evb   - Event builder description object.
#
proc ::Serialize::_saveEventBuilder {api evb} {
    
    #  Get the name, host and ring -- which are creation parameters:
    
    set props [$evb getProperties]
    set pos   [$evb getPosition]
    foreach var [list name host ring] {
        set $var [[$props find $var] cget -value]
    }
    #  The remainder of the properties are keys in the creation options dict:
    
    set opts [dict create]
    foreach prop [list servicePrefix serviceSuffix coincidenceInterval build timestampPolicy sourceId] \
            key  [list prefix        suffix        dt                  build tsPolicy        sourceId] {
        dict set opts $key [[$props find $prop] cget -value]
    }
    
    #  Create the evb:
    
    $api createEventBuilder $name $host $ring $opts
    $api evbSetEditorPosition $name [lindex $pos 0] [lindex $pos 1]
}
##
# ::Serialize::serializeEventBuilders
#    Save a list of event builders to file.
#    Note that any pre-existing event builders are destroyed.
#
# @param uri   - URI describing how to connect with the database.
# @param evbs  - List of event builder definition objects.
#
proc ::Serialize::serializeEventBuilders {uri evbs} {
    ::nscldaq::evb create _evbApi $uri
    _evbApi createSchema
    
    ::Serialize::_destroyEventBuilders _evbApi
    
    foreach evb $evbs {
        ::Serialize::_saveEventBuilder _evbApi $evb
    }
    
    ::nscldaq::evb destroy _evbApi
}
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
# @file dsourceSerializer.tcl
# @brief Serialization of data sources.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide dsourceSerializer 1.0
package require vardbEventBuilder
package require dataSourceObject

##
#  @note procs with names that start with _ after the namespace are
#        considered private

# Establish the Serializer namespace:

namespace eval ::Serialize {
    namespace export serializeDataSources deserializeDataSources
}

##
# ::Serialize::_saveSource
#
#   Save one data source to file.
#   @note - if the data source has no event builder for now it's not saved.
#           TODO: figure out how to save data sources that are not yet
#                 hooked up to event builders.
#
# @param api    - Api command word.
# @param ds     - data source object command.
#
proc ::Serialize::_saveSource {api ds} {
    set evb [$ds getEventBuilder]
    
    if {$evb ne ""} {
        set evbName [[[$evb getProperties] find name] cget -value]
    } else {
        set evbName "";            # Ronin source.
    }
    set props [$ds getProperties]
    
    # Get the properties that are creation arguments.
    
    foreach prop [list name host path ring ids] {
       set $prop [[$props find $prop] cget -value]   
    }
    
    # Get the properties that are in the options dict:
    
    set opts [dict create]
    foreach key [list info expectBodyHeaders defaultId timestampExtractor] {
       dict set opts $key [list [[$props find $key] cget -value] ]
    }
     
     # Create the data source.
     
    $api addSource $evbName $name $host $path $ring  $ids $opts
    # Save its position.
    set pos [$ds getPosition]
    
    #  A bit of a trick to save positions for ronin sources:
    
    if {$evbName eq ""} {
        set evbName " ";                  #directory it's really in.
    }
    
    $api dsSetEditorPosition $evbName $name [lindex $pos 0] [lindex $pos 1]
    
}

##
# ::Serialize::serializeDataSources
#    Serializes (saves) a set of data sources.  We are going to assume the
#    data source slate is clean which is true when this is used by the experiment
#    editor as it first saves event builders and that destroys event builders
#    and their data sources.
#
# @param uri    - Specifies the connection to the database.
# @param sources - List of data source description objects.
#
proc ::Serialize::serializeDataSources {uri sources} {
    ::nscldaq::evb create _dsApi $uri
    
    foreach source $sources {
        ::Serialize::_saveSource _dsApi $source
    }
    ::nscldaq::evb destroy _dsApi
}
##
# ::Serialize::deserializeDataSources
#    Reads the set of data sources from an experiment database:
#
# @param dburi   - URI that specifies how to connect with the database.
# @return list of dicts - each dict contains the following information:
#         * object - object that can be cloned onto the editor canvas to restore
#                    the graphical representation and state of the data source.
#         * x,y    - Desired coordinates for the event builder on the canvas
#         * evbName - Name of event builder this source sends data to.
#
proc ::Serialize::deserializeDataSources dburi {
    ::nscldaq::evb create _dsApi $dburi
    set result [list]
    
    foreach evb [_dsApi evbList] {
        set evbName [dict get $evb name]
        foreach ds [_dsApi listSources $evbName] {
            set name [dict get $ds name]
            set x    [_dsApi dsGetEditorXPosition $evbName $name]
            set y    [_dsApi dsGetEditorYPosition $evbName $name]
            
            set o [DataSourceObject %AUTO%]
            set props [$o getProperties]
            
            foreach p [list \
                    name host path info ids ring defaultId timestampExtractor \
                    expectBodyHeaders]                  {
                [$props find $p] configure -value [dict get $ds $p]
            }
            lappend result [dict create object $o x $x y $y evbName $evbName]
        }
    }
    
    ::nscldaq::evb destroy _dsApi
    return $result
}
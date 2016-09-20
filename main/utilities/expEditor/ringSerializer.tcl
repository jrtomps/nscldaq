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
# @file ringSerialize.tcl
# @brief Serialization of rings to db and back.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ringSerializer 1.0
package require vardbringbuffer
package require ringBufferObject

##
# @note Procs whose names (after the namespace) begin with _ are considered
#       private.


# Make sure the ::Serialize:: namespace exists:

namespace eval ::Serialize {
    namespace export serializeRings deserializeRings
}

##
# ::Serialize::_deleteRingBuffers
#
#  Delete all ring buffers in the database.
#
# @param api - API command basename attached to the database we care about.
#
proc ::Serialize::_deleteRingBuffers api {
    set rings [$api list]
    foreach ring $rings {
        $api destroy [dict get $ring name] [dict get $ring host]
    }
}
##
# ::Serialize::_saveRing
#    Create the database entries for a ringbuffer object.
#
# @param api    - Command base word for the database ring api
# @param ring   - A ringbuffer editor object.
#
proc ::Serialize::_saveRing {api ring} {
    
    # Ring buffers objects only support the default attributes of name, and host.
    # We also want to save the editor position so we can get it back later.
    
    set props [$ring getProperties]
    
    set host [[$props find host] cget -value]
    set name [[$props find name] cget -value]
    set pos  [$ring getPosition]
    
    $api create $name $host
    $api setEditorPosition $name $host [lindex $pos 0] [lindex $pos 1]
}
##
# ::Serialize::serializeRings
#
#    Save the ring buffer definitions to the vardb database.  Wer'e first going
#    to purge the existing definitions so that any deletions are also reflected.
#
# @param uri   - URI of the database connection.
# @param rings - Ring objects to serialize.
#
proc ::Serialize::serializeRings {uri rings} {
      ::nscldaq::vardbringbuffer create _ringApi $uri
      
      ::Serialize::_deleteRingBuffers _ringApi
      
      #  Now serialize the rings:
      
      foreach ring $rings {
        ::Serialize::_saveRing _ringApi $ring
      }
      
      ::nscldaq::vardbringbuffer destroy _ringApi
}

##
# ::Serialize::deserializeRings
#
#  Reads the set of ring buffer definitions from a program database file and
#  returns a list of dicts that allow the editor to restore them to the
#  canvas and its internal state:
#
# @param dburi - URI that specifies how to connect to the database.
# @return list of dicts containing the following keys:
#         * object - object that can be cloned onto the editor canvas.
#         * x,y    - Desired position of the graphrep of the object on the canvas.
#
proc ::Serialize::deserializeRings dburi {
    ::nscldaq::vardbringbuffer create _ringApi $dburi
    set result [list]
    
    foreach ring [_ringApi list] {
        set x [_ringApi getEditorXPosition [dict get $ring name] [dict get $ring host]] 
        set y [_ringApi getEditorYPosition [dict get $ring name] [dict get $ring host]] 
        
        # Make the editor object and fill in its properties:
        
        set obj [RingBufferObject %AUTO%]
        set p   [$obj getProperties]
        foreach prop [list name host] {
            [$p find $prop] configure -value [dict get $ring $prop]
        }
        #  Create/add the dict to the result:
        
        lappend result [dict create object $obj x $x y $y]
    }
    
    ::nscldaq::vardbringbuffer destroy _ringApi
    return $result
}
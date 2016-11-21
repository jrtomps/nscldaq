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
# @file LogModel.tcl
# @brief Model for log messages.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide LogModel

##
# This package provides a model for {set log messages.
# It maintains a database of LOG messages.  The log message database is
# created as follows:
#   *  If -file is a file then the database is created in the file -- which
#      can exit in order to support a persistent database of log messages.
#   *  If -file is blank, the database is created in a temporary file
#      deleted on destruction of this type
#   *  IF -file is :memory: the datagse is in memory.
#
#

package require sqlite3
package require snit

##
# @class LogModel
#    Snit class that encapsulates the log file.
#

snit::type LogModel {
    option -file -default [
                           list]
    variable dbCommand
    
    constructor args {
        $self configurelist $args
        set dbCommand [$self _createDatabase]
        $self _createSchema
    }
    
    #destructor {
    #    $dbCommand close
    #}
    ##
    #  API:
    #
    
    ##
    # addMessage
    #   Add a log message to the table.
    # @param msg - a decoded log message. It is an error to pass any message
    #              type other than LOG_MESSAGE
    #
    method addMessage {msg} {
        set header [lindex $msg 0]
        set body   [lindex $msg 1]
        
        set type [dict get $header type]
        
        if {$type ne "LOG_MESSAGE"} {
            error "Wrong message type should be LOG_MESSAGE was $type"
        }
        
        #  Pull the bits and pieces we need:
        
        set severity [dict get $header severity]
        set app      [dict get $header application]
        set source   [dict get $header source]

        set tod      [dict get $body timestamp]
        set msg      [dict get $body message]
        
        $dbCommand eval {INSERT INTO log_messages
            (severity, application, source, timestamp, message)
            VALUES ($severity, $app, $source, $tod, $msg)
        }
    }
    method trim {criterion} {
        
    }
    method get {{filter {}} } {
    }
    method count {} {
        
    }
    #--------------------------------------------------------------------------
    #  Private methods.
    #
    
    ##
    # _createDatabase
    #    Create a new database 
    #
    # @return name of the database command.
    #
    method _createDatabase {} {
        sqlite3 ${selfns}::db $options(-file)
        return ${selfns}::db
    }
    ##
    # _createSchema
    #   Creates the database schema.
    #   Each log message has the following fields:
    #   *  id - auto increment primary key.
    #   *  severity - Severity of the message (indexed).
    #   *  application - Name of createing application (indexed)
    #   *  source      -  Source host of message (indexed)
    #   *  timestamp   -  UNIX timestamp of message (indexed)
    #   *  message     -  Text of the message.
    #
    #  @note $dbCommand is assumed to hold the database command.
    #
    method _createSchema {} {
        
        #  Raw schema with primary key:
        
        $dbCommand eval "CREATE TABLE IF NOT EXISTS log_messages (   
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            severity     TEXT(10)                          ,
            application  TEXT(32)                         ,
            source       TEXT(128)                        ,
            timestamp    INTEGER                          ,
            message      TEXT     
        )"
        #  Add indices:
        
        $dbCommand eval "CREATE INDEX IF NOT EXISTS 
            idx_log_severity ON log_messages (severity)    
        "
        $dbCommand eval "CREATE INDEX IF NOT EXISTS 
            idx_log_application ON log_messages (application)
        "
        $dbCommand eval "CREATE INDEX IF NOT EXISTS 
            idx_log_source ON  log_messages (source)
        "
        $dbCommand eval "CREATE INDEX IF NOT EXISTS 
            idx_log_timestamp ON log_messages (timestamp)
        "
    }
    
}

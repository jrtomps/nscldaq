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
package provide LogModel 1.0

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
    ##
    # Trims the log_messages table of old messages.
    #
    # @param criterion - two element keyword value list that specifies how to trim:
    #               *  keep n - Keep only the most recent n records.
    #               *  since date/time - Keep only those records as new
    #                               or newer than date/time - date/time is anything
    #                               [clock scan] can handle.
    # The messages that don't meet the criterion supplied are deleted from the
    # table.
    method trim {criterion} {
        # Criterion must be a two element list:
        
        if {[llength $criterion] != 2} {
            error "trim criterion must be a two element list"
        }
        set whereClause [$self _trimCriterionToWhereClause $criterion] 
        
        
        $dbCommand eval "delete from log_messages $whereClause"
    }
    ##
    # get
    #   Get log records from the database in accordance with the filter
    #   criteria.
    #
    # @param filter - describes any filter criteria as a list of sublists;  An empty
    #                 filter implies no filtering.  Each sublist contains a
    #                 fieldname followed by a relational operator followed by a value.
    #                 The 'timestamp' field is handled specially in that it is
    #                 [clock scan]ned
    #                 
    # @return list of dicts with the key value pairs (note these are field names):
    #         *  severity - message severity
    #         *  application - Application name
    #         *  source      - data source.
    #         *  timestamp   - Timestamp.
    #         *  message     - Message string.
    #
    method get {{filter {}} } {
        set whereClause [$self _getFilterToWhereClause $filter]
        puts $whereClause
        set result [list]
        $dbCommand eval "SELECT severity, application, source, timestamp, message \
                            FROM log_messages $whereClause ORDER BY id ASC" record {
            set row [dict create]
            foreach key [list severity application source timestamp message] {
                dict set row $key $record($key)
            }
            lappend result $row
        }
        return $result
    }
    ##
    #  Return the number of messages that are in the database.
    #  This is intended mostly for in memory databases which may need to be
    #   trimmed to prevent virtual memory exhaustion.
    # @return integer - number of messages in the database.
    #
    method count {} {
        return [$dbCommand eval {SELECT COUNT(*) FROM log_messages}]    
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
    ##
    # _trimCriterionToWhereClause
    #    Convert the trim criterion to a delete where clause.  Note that
    #    the criterion actually specifies what do retain.
    #
    # @param criterion - trim criteriion.  See the trim method for information
    #                    about this.
    #
    method _trimCriterionToWhereClause {criterion} {
        set how  [lindex $criterion 0]
        set what [lindex $criterion 1]
        
         if {$how eq "keep"} {
            if {![string is integer -strict $what]} {
                error "keep criterion requires an integer value: '$what'"
            }
            if {$what <= 0} {
                error "keep criterion requires $what > 0"
            }
            # Keeping the most recent n requires getting a list of their ids
            # and generating a NOT IN clause from them to describe what is
            # to be deleted:
            
            set keptIds [list]
            $dbCommand eval \
                "SELECT id FROM log_messages ORDER BY id DESC LIMIT $what" record {
                lappend keptIds $record(id)
            }
            set whereClause "WHERE id NOT IN ([join $keptIds {, }])"
            return $whereClause
         } elseif {$how eq "since"} {
            set timestamp [clock scan $what]      ;# Hopefully  errors on bad times.
            set whereClause "WHERE timestamp >= $timestamp"
            return $whereClause
         } else {
            error "Criterion must be a since or keep item instead of $how"
         }
         
    }
    ##
    # _getFilterToWhereClause
    #   Turns a filter for the get method into an SQL WHERE clause.
    #
    #  Filter clauses are of the form {filename binop value}  e.g.
    #  {application ==  'george'} is a filter clause that requires
    #  messages be from the george application.  Note that the
    #  binop can be any valid SQLITE binary operator e.g:
    #  {application IN ('a','b','c')} filters to applications that are
    #  any of a,b,c -- though this is not anticipated to be the normal use.
    #
    # @param filter - Filter specification (list of sublists).
    # @return string a WHERE clause (note that an empty filter spec returns
    #                  nothing which does not filter any data).
    #
    method _getFilterToWhereClause filter {
        if {[llength $filter] == 0} {
            return "";                 # No Filter.
        }
        #  Build up a list of WHERE subclauses from the filter criteria.
        #  We're on the lookout for timestamp whose value must be treated specially:
        #
        set clauses [list]
        foreach spec $filter {
            set field [lindex $spec 0]
            set op    [lindex $spec 1]
            set value [lindex $spec 2]
            if {$field eq "timestamp"}  {
                set value [clock scan $value];   # Values are stringed dates.cd /
            }
            # If values are a list turn them into ('v1','v2') ...
            
            if {[llength $value] > 1} {
                set valueList [list]
                foreach v $value {
                    lappend valueList '$v'
                }
                set value "([join $valueList ", "])"
            } else {
                set value '$value'
            }
            lappend clauses "$field $op $value"
        }
        # Join the clauses with an AND:
        
        set whereClause [join $clauses " AND "]
        return "WHERE $whereClause"
    }
}   


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
# @file logtest.tcl
# @brief Simple test program for logging.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# This is a simple test program for a log display.
# We create a log display object
# We start multinode aggregation as an internal thread.
# We subscribe to log messages.
# We attach a script to log message receipt that decodes the message
# and dispatches it to the log display object.

package require LogDisplay
package require statusMessage
package require dialogWrapper

proc processMessage {rawMessage} {
    set decodedMessage [statusdecode $rawMessage]
    $::logDisplay addMessage $decodedMessage
}

proc updateFilter {} {
    set filters [list]    
    # Severity selection severity in (all severities with value 1)
    
    set severitiesShown [list]
    foreach severity [list DEBUG INFO WARNING SEVERE DEFECT] {
        
        if {[set ::$severity]} {
            lappend severitiesShown $severity
        }
    }
    if {[llength $severitiesShown] > 1} {
        set sevFilter [list severity in $severitiesShown]
        lappend filters $sevFilter
    } else  {  
        set sevFilter [list severity = $severitiesShown]
        lappend filters $sevFilter
    }
    # Application filter:
    
    if {$::visibleApplication ne""} {
        lappend filters [list application == $::visibleApplication]
    }
    #  Source filter:
    
    if {$::visibleSource ne ""} {
        lappend filters [list source == $::visibleSource]
    }

    $::logDisplay setFilter $filters
}


proc setTextFilter which {
    upvar $which value
    toplevel .d
    DialogWrapper .d.prompt -showcancel 1
    set c [.d.prompt controlarea]
    entry $c.e
    $c.e insert end $value
    .d.prompt configure -form $c.e
    pack .d.prompt
    set result [.d.prompt modal]
    if {$result eq "Ok"} {
        set value [$c.e get]
        updateFilter    
    }
    destroy .d
    
    
    
}

proc clearFilters {} {
    foreach severity [list DEBUG INFO WARNING SEVERE DEFECT] {
        set ::$severity 1
    }
    set ::visibleApplication ""
    set ::visibleSource ""
    updateFilter
}

# Set up log display menu handling

set logDisplay [LogDisplay %AUTO% -filename test.db]
pack .logview
set uri [statusaggregator]
    
set sub [statusSubscription create $uri [list [list LOG_MESSAGE {}]]]
$sub onMessage processMessage

# Set up menus for filtering

menu .menubar 
menu .menubar.filter -tearoff 0
.menubar add cascade -label Filter -menu .menubar.filter

. configure -menu .menubar

# Populate the filter menu:

# Severity filtering;  Variables for each of the severity types:

set DEBUG   1
set INFO    1
set WARNING 1
set SEVERE  1
set DEFECT  1

set visibleApplication ""
set visibleSource      ""

.menubar.filter add command -label {Filter Severities:}
foreach sev [list DEBUG INFO WARNING SEVERE DEFECT] {
    .menubar.filter add checkbutton -onvalue 1 -offvalue 0 -variable $sev \
        -label $sev -command updateFilter
}
.menubar.filter add separator
.menubar.filter add command -label Application... -command [list setTextFilter visibleApplication]
.menubar.filter add command -label Source...      -command [list setTextFilter visibleSource]

.menubar.filter add separator
.menubar.filter add command -label Clear -command [list clearFilters]

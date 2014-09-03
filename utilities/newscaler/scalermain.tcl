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
# @file scalermain.tcl
# @brief Scaler program main.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide scalermain 1.0


if {[info command __package_orig] eq "__package_orig"} {

    # pkg_mkIndex is running us so exit before some of our esoteric stuff fails.
    # Making this package not index properly.
    
    return
}

set here [file dirname [info script]]
set libdir [file normalize [file join $here ..]]
lappend auto_path $libdir


package require Thread
package require Tk
package require scalerconfig
package require header
package require scalerReport

## Ensure the SCALER_RING env variable is defined

if {[array names  ::env SCALER_RING] eq ""} {
    tk_messageBox -icon error -type ok -title "No ring" \
    -message "Define the SCALER_RING environment variable to be the URI of the ringbuffer"
    exit -1
}

## Ensure the user supplied a configuration file:

if {[llength $argv] != 1} {
    puts stderr "You need to supply a single parameter: the configuration script filename."
    exit -1
}

##
# Globals:

set notebook "";                  # Widget containing the tabbed notebook.
set header   "";                  # Header widget.

set startTime 0;                  # when the run started:

##
# Create the thread that will read data from the ring and post it back to us:
#
proc startAcqThread {} {
    set acqThread [thread::create -joinable]
    puts $::libdir
    if {[thread::send $acqThread [list lappend auto_path $::libdir] result]} {
        puts "Could not extend thread's auto-path"
        exit -1
    }
    if {[thread::send $acqThread [list package require TclRingBuffer] result]} {
        puts "Could not load RingBuffer package in acqthread: $result"
        exit -1
    }
    
    if {[thread::send $acqThread [list ring attach $::env(SCALER_RING)] result]} {
        puts "Could not attach to scaler ring buffer in acqthread $result"
        exit -1
    }
    
    #  The main loop will forward data to our handleData item.
    
    set myThread [thread::id]
    set getItems "proc getItems {tid uri} { 
        while 1 {                                             
            set ringItem \[ring get \$uri {1 2 20}]             
            thread::send \$tid \[list handleData \$ringItem]     
        }                                                     
    }                                                         
    getItems $myThread $::env(SCALER_RING)                    
    "
    thread::send -async $acqThread $getItems

    
    return $acqThread
}
##
#  updatePages
#    Updates all pages on the display.
#
proc updatePages {} {

    set tabNames [::scalerconfig::pages list]
    foreach tab $tabNames {
        set widget [::scalerconfig::pages get $tab]
        $widget update
    }
    
}
##
# scaler
#   Called when a scaler item comes in.
#   * Figure out if there is a data source.
#   * For each scaler figure out the identifier
#   * If there is a command corresponding to the identifier,
#     update the scaler.
#   * Update all display widgets.
# @param item - The scaler item dict.
#
proc scaler item {
    # puts $item
    #
    #  Figure out if there's a source id that needs to appended to the channel
    #  number:
    
    if {[dict exists $item bodyheader]} {
        set sourceId .[dict get $item bodyheader source]
    } else {
        set sourceId "";                   # No source id.
    }
    # Figure out the interval over which the scalers accumulated
    # Note the divisor changes whatever timebase used for the start/end
    # values to floating point seconds.
    
    set end [dict get $item end]
    set start [dict get $item start]
    set dt [expr {double($end - $start)/[dict get $item divisor]}]
    
    # Iterate over the channels invoking update methods for those channels
    # that have been defined for us:
    
    set channel 0
    foreach counter [dict get $item scalers] {
        if {[info command ::channel_$channel$sourceId] ne ""} {
            ::channel_$channel$sourceId update $counter $dt
        }
        
        incr channel
    }
    
    updatePages
    
    # The state is now known to be active (if it was not known before):
    
    set h [getHeader]
    $h configure -state Active
    
}
##
# beginRun
#   Handles begin run items:
#   *   All counters are reset.
#   *   The header run number and title are  reset.
#   *   The state is set to Active.
#   *   All pages are refreshed.
#
# @param item - The dict that contains the begin run item.
#
proc beginRun {item} {
    
    set ::startTime [dict get $item realtime]
    
    foreach counter [::scalerconfig::channelMap list] {
        [::scalerconfig::channelMap get $counter] clear
        
    }
    updatePages
    
    set h [getHeader]
    $h configure -title [dict get $item title] -run [dict get $item run] \
        -state Active
}
##
# endRun
#   state -> inactive.
proc endRun   {item} {
    
    [getHeader] configure -state Inactive
    
    # Only make reports if the run start time is available:
    
    if {$::startTime > 0} {
        
        
        # Human readable:
        
        set run [dict get $item run]
        set filename [format run%04d.report $run]
        set fd [open $filename w]
        humanReport $fd $::startTime $item ::scalerconfig::channelMap
        close $fd
        
        #  Computer readable:
        
        set filename [format run%04d.csv $run]
        set fd [open $filename w]
        computerReport $fd $::startTime $item ::scalerconfig::channelMap
        close $fd
    }
}

##
# handleData
#   Called with a dict that contains the ring item when a new ring item arrives.
#
# @param item - the new ring item.
#
proc handleData item {
    #puts $item
    # Dispatch based on the type of event:
    
    set type [dict get $item type]
    switch $type {
        "Begin Run" {beginRun $item}
        "End Run"   {endRun   $item}
        "Scaler"    {scaler   $item}
    }
}

# Procs to help the configuration file interact with the GUI:

##
# getNotebook
# @return widget path of the notebook.
#
proc getNotebook {} {
    return $::notebook
}
##
# Add a new widget to the notebook.
#
# @param widget - path to the widget - must be a child of the notebook.
# @param tabname - Text to put in the tab.
#
proc addPage {widget tabname} {
    pack $widget -fill both -expand 1;    # Just stack them in the toplevel for now.
    [getNotebook] add $widget -text $tabname
}

##
# getHeader
#   Return the header widget.
#
proc getHeader {} {
    return $::header
}

##
# setupGui
#   Set up the top level gui stuff.
#
proc setupGui {} {
    set ::header [header .header -title ????? -run ????]
    pack .header -fill x -expand 1
    set ::notebook [ttk::notebook .notebook]
    pack .notebook -fill both -expand 1    
}
#-----------------------------------------------------------------------------
# Main script entry point.

# Start the acquisition thread it will post an event for handleData when items
# of interest are seen in the ring.

set acqThread [startAcqThread]

# Set up the base graphical user interface:


setupGui


#
# Process the scaler configuration file

set configFile [lindex $argv 0]

source $configFile


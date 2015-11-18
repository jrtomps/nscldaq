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
# @file RingStatus.tcl
# @brief Snit megawidget to display the status of a ring buffer.
# @author Ron Fox <fox@nscl.msu.edu>
#

package require Tk
package require snit



package provide RingStatus 1.0

##
# @class  RingStatus
#    Megawidget that displays the status of a ring buffer.
#    Note that this is a view only.  The only coupling
#    with ringbuffers is that the format of the data used to update
#    the widget state is the same as that retuned by the ring package's
#    'ringbuffer usage' subcommand.
#
#  LAYOUT:
#     +------------------------------------------------+
#     |                    <ring name>                 |
#     | Size: <ring data size>  Free: <Put bytes>      |
#     |                    Consumer usage:             |
#     |    pid                bytes queued             |
#     |    n                   xxxxxx                  |
#     |    ...                  ...                    |
#     +------------------------------------------------+
#
# OPTIONS:
#   -name - Name of the ring.
# METHODS:
#   update   Update the view.
#
snit::widgetadaptor RingStatus {
    component consumers

    option -name
    
    
    #  Indexed by consumer PID the contents are the treeview item id
    #  of the treeview entry for that consumer.
    
    variable rows -array [list]
    
    ##
    # Construct the widget.  note, the pid list is a treeview.
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.name -textvariable ${selfns}::options(-name)
        ttk::label $win.sizelbl -text Size:
        ttk::label $win.size    -text {}
        ttk::label $win.freelbl -text Free:
        ttk::label $win.free    -text {}
        
        grid x  $win.name -columnspan 2
        grid $win.sizelbl $win.size $win.freelbl $win.free -sticky w
        
        set consf [ttk::labelframe $win.consf -text {Consumer Usage:}]
        install consumers using ttk::treeview $consf.tree -show headings \
            -yscrollcommand [list $consf.sb set]
        ttk::scrollbar $consf.sb -orient vertical -command [list $consumers yview]
        $consumers configure -columns [list pid {Bytes queued}] -selectmode none
        $consumers heading pid -text Pid
        $consumers heading 1   -text {Bytes Queued}
        
        grid $consumers $consf.sb -sticky nsew
        grid $consf -sticky nsew -columnspan 4
        
        $self configurelist $args
    }
    
    ##
    # update
    #    Update the ring item display.
    #  @param usage - List in the format from [ringbuffer usage]:
    #                *  Ring buffer size.
    #                *  Ring buffer free space.
    #                *  Maximum consumers allowed.
    #                *  producer - Pid of the producer.
    #                *  maxget   - Biggest chunk that can be gotten by least caught up.
    #                *  minget   - Biggest chunkthat can be gotten by most caught up.
    #                *  List of pairs for each consumer containing the PID and bytes queued
    #                   for that consumer.
    #
    method update usage {
        $win.size configure -text [lindex $usage 0]
        $win.free configure -text [lindex $usage 1]
        
        set pids [list]
        
        # Update/add lines to the treeview.
        
        foreach item [lindex $usage 6] {
            set pid     [lindex $item 0]
            set backlog [lindex $item 1]
            lappend pids $pid
            
            set item [$self _itemId $pid]
            $consumers item $item -values [list $pid $backlog]
            
        }
        # Remove lines for consumers that have vanished.
        
        
        foreach pid [array names rows] {
            if {$pid ni $pids} {
                $consumers delete $rows($pid)
                array unset rows $pid
            }
        }
    }
    
    #-----------------------------------------------------------------------
    # private methods
    #
    
    ##
    # _itemId
    #   Get the id of an item given its PID.  Note that if the item does
    #   note yet exist, it is created.
    #
    # @param pid - Pid to lookup.
    # @return string - Item id of the item for that PID.
    #
    method _itemId pid {
        if {[array names rows $pid] eq ""} {
            set rows($pid) [$consumers insert {} end -values [list $pid]]
        }
        return $rows($pid)
    }
}
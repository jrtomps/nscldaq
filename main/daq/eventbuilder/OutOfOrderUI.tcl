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
# @file OutOfOrderUI.tcl
# @brief UI to display out of order inputs.
# @author Ron Fox <fox@nscl.msu.edu>
#

package require Tk
package require snit
package require csv

package provide EVB::OutOfOrderUI 1.0


##
# @class OODisplay
#   Megawidget consisting of a treeview and scrollbar(s).  The treeview
#   contains the following columns:
#   *   sourceId      - These are tree parents.
#   *   Count         - Number of events for each source.
#   *   Time          - When the event occured.
#   *   LastTimestamp - Prior timestamp.
#   *   BadTimestamp  - Out of order timestamp.
#
#  So you can imagine a tree view might look like:
#
# \verbatim
#   SourceId      Count      Time              LastTimestamp     BadTimestamp
#      1            2
#      +---             2014-12-18 08:12:34     124567               666
#                       2014-12-18 09:34:12    7654321             12345
#
# \endverbatim
#   Thus each top level node is a source id and the number of events it tallies.
#   it's child nodes are the events themselves timestamped with the date/time
#   they were detected.
#
#  METHODS:
#    add   - Add a new event.
#    clear - Clear the events.
#    write - Write the events as a to file in XCSV form (sid,time,lastts,badts).
#
#  OPTIONS:
#    All options are delegated to the treeview..but beware since that does mean you can
#    screw things up (normally you'll just use this for -height).
#
snit::widgetadaptor OutOfOrderUI {
    component tree
    option    -retain -default 2000
    option    -trimdivisor -default 10
    
    delegate option * to tree

    # inded by source id, contents are the item number assigned to that top level
    # item.
    
    variable ids -array [list]
    variable columnNames [list count time {Last Stamp} {Bad Stamp}]
    
    ##
    # constructor
    #  Create the hull (ttk::frame) the tree view and the scroll bar (vertical).
    #  Lay them all out.
    #
    #  Process any configuration options in args
    #
    constructor args {
        installhull using ttk::frame
        
        install tree using ttk::treeview $win.tree \
            -yscrollcommand [list $win.sb set]
        ttk::scrollbar $win.sb -orient vertical -command [list $tree yview]
        
        grid $tree $win.sb -sticky nsew
        
        $self _configureTreeView
        $self configurelist $args
    }
    
    
    ##
    # add
    #   Adds a new item.  If the item's id is not yet a tree item
    #   it is added.
    #
    # @param id     - Id of the item to add.
    # @param time   - [clock seconds] of the time to add
    # @param prior  - Prior timestamp.
    # @param bad    - Bad timestamp.
    #
    #  The count data for the id are updated.
    #
    method add {id time prior bad} {
        set itemId [$self _getIdItem $id];               # Create if needed.
        $self _incrementCount $itemId
        
        set timeString [clock format $time -format "%Y-%m-%d %R:%S %Z"]
        
        $tree insert $itemId end -values [list "" $timeString $prior $bad]
        
        # If we have too many items, then we must delete the first few of them:
        # We rely on the fact that the 
        set items [$tree children $itemId]
        if {[llength $items] > $options(-retain)} {
            set numToKill [expr {$options(-retain)/$options(-trimdivisor)}]
            set killItems [lrange $items 0 $numToKill]
            $tree delete $killItems
        }
    }
    
    ##
    # clear
    #   Remove all entries.
    #
    method clear {} {
        #  Delete the children of each id:
        
        set toplevelItems [list]
        foreach id [array names ids] {
            $tree delete [$tree children $ids($id)]
            lappend toplevelItems $ids($id)
        }
        
        # delete the toplevel nodes.
        
        $tree delete $toplevelItems
        
        # Kill off the array elements.
        
        array unset ids *
        
    }
    
    ##
    # write
    #   Write a csv version of the tree.  The file has columns:
    #    id,time,laststamp,badstamp
    #
    # @param fd - File descriptor open on the file to write too.
    #
    method write fd {
        foreach id [array names ids] {
            set children [$tree children $ids($id)]
            foreach child $children {
                set info [$tree item $child -values]
                set time [lindex $info 1]
                set prior [lindex $info 2]
                set bad   [lindex $info 3]
                
                puts $fd [csv::join [list $id $time $prior $bad]]
            }
        }
    }
    #---------------------------------------------------------------------------
    #  Private methods
    #
    
    ##
    # _getIdItem
    #   Gets the item identifier for a source id.  If the source does
    #   not yet correspond to a tree node one is created.
    #
    # @param id  - source id.
    # @return string - treeview id of the source id's node.
    #
    method _getIdItem id {
        if {[array names ids $id] eq "" } {
            set ids($id) [$tree insert {} end -text $id -values [list 0]]
        }
        return $ids($id)
    }
    ##
    # _incrementCount
    #   Increment the counter of items for a sourcde id
    #
    # @param item - item identifier to increment.
    #
    method _incrementCount item {
        set count [lindex [$tree item $item -values] 0]
        incr count
        $tree item $item -values [list $count]
        
    }
    
    ##
    # _configureTreeView
    #
    #   Sets up the characteristics of the tree view
    # 
    method _configureTreeView {} {
        $tree configure -columns $columnNames
        $tree configure -displaycolumns #all -selectmode none -show [list tree headings]
        foreach name $columnNames {
            $tree column $name -stretch 1 -anchor w
            $tree heading $name -text $name -anchor w
        }
        $tree heading #0  -text id
    }
}
##
# @class OutOfOrderWindow
#    This is an OutOfOrderUI that has an additional pair of buttons below it:
#    *   Clear clears the contents of the window.
#    *   Save... prompts for a file to which to write the contents of the window.
#
#   The add, clear, write methods of the OutOfOrderUI are exposed as are all of its
#   tree view options.
#
#
snit::widgetadaptor OutOfOrderWindow {
    component tree
    component clear
    component save
    
    delegate option * to tree
    delegate method add to tree
    delegate method save to tree
    delegate method clear to tree
    
    delegate method * to hull
    
    
    ##reta
    # construtor
    #    Put everthing together.
    #
    constructor {args} {
        installhull using ttk::frame
        
        install tree using OutOfOrderUI $win.tree
        ttk::frame $win.buttons
        install clear using ttk::button $win.buttons.clear \
            -text Clear -command [list $tree clear]
        install save using ttk::button $win.buttons.save \
            -text Save... -command [mymethod _save]

            
        grid $tree -sticky nsew
        grid $clear $save -sticky w -padx 5
        grid $win.buttons -sticky nsew
        
        $self configurelist $args
            
    }
    
    method _save {} {
        set file [tk_getSaveFile -title {Choose save file} \
            -filetypes [list                               \
               {{CSV Files} .csv }                        \
               {{All files}  *     }                      \
            ]                                             \
        ]
        if {$file ne ""} {
            set fd [open $file "w"]
            $tree write $fd
            close $fd
        }
    }
}
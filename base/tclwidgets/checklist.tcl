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
# @file checklist.tcl
# @brief Widget for checklists.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide checklist
package require Tk
package require snit

##
# @class checklist
#    Provides a class for checklists.  A checklist is a widget that contains
#    checkboxes and text elements:
#
#    * Checking the checkbox overstrikes the text associated with it.
#    * Unchecking the checkbox removes the overstrike.
#    * The client can query the items in the checklist.
#    * The client can query the items not checked off.l
#    * The client can query the checked off items.
#    * A button "Remove Completed"  Removes all checked off items from the
#      checklist (those items are not forgotten).
#    * The client can query the 'hidden' items.
#
# METHODS:
#
#   *  addItem - add a checklist item.
#   *  getItems - Get all items.
#   *  getCompleted Return all checked off items.
#   *  getTodo - Return all items that are left to do.
#   *  getHidden - Return the items that are hidden.
#
snit::widgetadaptor checklist {
    variable allItems    [list]
    variable doneItems   [list]
    variable hiddenItems [list]

    variable widgetIndex 0
    
    ##
    # constructor
    #    Install the hull.
    #    There are no options to configure.
    #    Note that while we create the purge button, it's not
    #    going to be shown until there are some unchecked off items.
    #
    constructor args {
	installhull using ttk::frame
	button $win.purge -text {Remove Completed} -command [mymethod _purgeCompletedItems]

	# If our font was not created, create it:

	if {"osfixed" ni [font names]} {
	    font create osfixed -family TkDefaultFont
	    font config osfixed -overstrike 1
	}
    }
    #--------------------------------------------------------------------
    #  public methods:
    #

    ##
    #  addItem
    #     Add a new item to the checklist.
    # 
    # @param item - Text of the new item to add.
    #
    method addItem item {
	lappend allItems $item;		# Record it for retrieval.
	
	ttk::checkbutton $win.cb$widgetIndex -onvalue 1 -offvalue 0 -command [mymethod _toggle $widgetIndex]
	ttk::label       $win.tx$widgetIndex -text $item -font TkDefaultFont

	grid forget $win.purge
	grid $win.cb$widgetIndex $win.tx$widgetIndex -sticky w
	grid $win.purge -columnspan 2 -sticky w

	incr widgetIndex
    }
    
    #---------------------------------------------------------------------
    # Private methods.
    #

    ##
    # _toggle
    #   Responds to clicks on the check button:
    #   - The font of the text item is used to determine state and is toggled.
    #   - If we went from done to not done, we remove the item from the donItems list.
    #   - If we went from not done to done , we append the item to the done list.
    #
    # @param idx - Widget index from which the two widget names can be constructed.
    #
    method _toggle idx {
	set tw $win.tx$idx
	if {[$tw cget -font] ne "osfixed"} {
	    # Complete item:

	    $tw configure -font osfixed
	    lappend doneItems [$tw cget -text]
	} else {
	    # Uncomplete item:

	    $tw configure -font TkDefaultFont
	    set text [$tw cget -text]
	    set txIdx [lsearch -exact $doneItems $text]
	    set doneItems [lreplace $doneItems $txIdx $txIdx]
	}
    }
    ##
    # _purgeCompletedItems
    #    Removes completed items from the display.  Those that are not already
    #    in the hidden items list are added.
    #
    method _purgeCompletedItems {} {
	foreach widget [winfo children $win] {

	    # only worry about widgets of the form .....txn where n is a number
	    # These are the text widgets with the item strings.

	    set tail [lindex [split $widget .] end]
	    if {[string range $tail 0 1] eq "tx"} {

		# We only care about overstruck itesm:

		if {[$widget cget -font] eq "osfixed"} {
		    
		    # Get the text item kill of the checkbutton and the widget
		    # That's how we hide them.

		    set text [$widget cget -text]
		    set n    [string range $tail 2 end]
		    set cb $win.cb$n
		    destroy $widget
		    destroy $cb

		    # If the string is not in the hiddenItems list, add it:

		    if {$text ni $hiddenItems} {
			lappend hiddenItems $text
		    }
		}
	    }
	}
    }


}

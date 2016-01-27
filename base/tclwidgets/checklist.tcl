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
    
}

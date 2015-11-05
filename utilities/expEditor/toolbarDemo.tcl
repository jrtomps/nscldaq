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
# @file toolbarDemo.tcl
# @brief Demonstrate/exercise toolbars and tools.
# @author Ron Fox <fox@nscl.msu.edu>
#

set here [file dirname [info script]]
source [file join $here toolbar.tcl]

package require ringBufferObject
package require stateProgram
package require Service
package require objectInstaller
package require tool

# Make the tool bar:

toolbar .t -width 60 -height 512

# Make the target canvas, bind the toolbar to it and layout the ui:

canvas .c -width 512 -height 512
.t configure -target .c
grid .t .c -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 1 -weight 1

# Create and add some tools:

tool ring [RingBufferObject %AUTO%] [ObjectInstaller %AUTO%]
.t add ring

tool statePgm [StateProgram %AUTO%] [ObjectInstaller %AUTO%]
.t add statePgm

tool service [Service %AUTO%] [ObjectInstaller %AUTO%]
.t add service

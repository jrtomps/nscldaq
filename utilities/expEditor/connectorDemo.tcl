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
# @file connectorDemo.tcl
# @brief Exercise connectors.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  Make a donut
#  Make a program.
#  Enable motion on both the donut and the program.
#  Make a connector between the donut and program.
#
#  What should happen is that as the donut and program
#  are moved, the connector should maintain connection.
#
# TODO - figure out a smart way to move the end points around the circumscribing
#        rectangle rather than maintaining the fixed connection point.
#
package require Tk
package require img::png

set here [file dirname [info script]]
image create photo donut -format png -file [file join $here ringbuffer.png]
image create photo program -format png -file [file join $here program.png]

source [file join $here connector.tcl]

#  Make the canvas:
#

canvas .c -height 512 -width 512
pack .c  -fill both -expand 1

#
#  Put the program and donut on the vertical centerline.. a bit more than
#  their width in from the edge.
#  The attachment points will then be at the y of the icons but an X that is
#  the 'inner' edge  of the icons.
#
set y 256

set pw [image width program]
set px $pw

set dw [image width donut]
set dx [expr {512 - $dw}]

set pid [.c create image $px $y -image program]

set did [.c create image $dx $y -image donut]

#Figure out the connection x pts:

set fromx [expr {$px + $pw/2}]
set tox   [expr {$dx - $dw/2}]

set c [connector %AUTO% \
    -canvas .c -from $pid -to $did -fromcoords [list $fromx $y]   \
    -tocoords [list $tox $y] -arrow last]

puts [.c gettags $pid]
puts [.c gettags $did]

## Let the icons be movable:

.c bind $did <B1-Motion> [list .c coords $did %x %y]
.c bind $pid <B1-Motion> [list .c coords $pid %x %y]

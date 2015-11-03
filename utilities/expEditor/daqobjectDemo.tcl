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
# @file daqobjectDemo.tcl
# @brief Demonstrate a DAQObjecst.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require Tk
package require DaqObject
package require img::png

##
#  - Make a canvas
#  - Make a DAQObject on the canvas.
#  - Tag it as obj1.
#  - Bind <Motion> to move the object around the canvas.
#  - Bind double click to clone the object which is tagged with a different
#  - tagged with a different tag and has motion bound to it.
#

image create photo donut -format png -file ringbuffer.png
set ht [image height donut]
set w  [image width donut]

set x [expr {$ht/2 + 5}]
set y [expr {$w/2 + 5}]

#  Make the canvas.

canvas .c   -height 512 -width 512
pack .c

# Initial object


set obj1 [DaqObject %AUTO% -canvas .c -image donut]
$obj1 drawat $x $y

# Add obj1 tag to the object, ensure that it's still added. Bind a motion event
# to that tag that will move the object:

$obj1 addtag obj1
puts [$obj1 tags]
# .c bind obj1 <B1-Motion> [list drag $obj1 %x %y]
.c bind obj1 <Double-Button-1> [list duplicate $obj1 %x %y]



##
#  Note x y are absolute x, y
proc drag {object x y} {
    
    $object moveto $x $y
}


set cidx 1;        # Clone index.
#
#  * We'll offset the new object from x/y by a bit.
#  * We'll warp the pointer to the new x/y as well.
#
#  Key question will the motion now apply to the new object?
#
proc duplicate {object x y} {;
    
    
    # Place the object on the canvas.
    
    set newobj [$object clone]
    $newobj configure -canvas .c
    $newobj drawat $x $y
    
    # Move it a bit -- test the moveby function:
    
    $newobj moveby 5 5
    
    #  Bind motion:
    
    set ::obj[incr ::cidx] $newobj;    # Global name for it.
    $newobj addtag obj$::cidx
    .c bind obj$::cidx <B1-Motion> [list drag $newobj %x %y] 
}

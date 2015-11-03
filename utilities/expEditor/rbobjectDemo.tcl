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
# @file rbObjectDemo.tcl
# @brief Put RingBufferObjects through their paces.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# - Create a canvas and put a ring buffer object in the left upper corner.
# - Bind a double click on that object to clone a new one offset a bit.
# - Cloned objects are bound to drag.
# - Double clicking a cloned object brings up a property sheet editor dialog for it.
# - Hovering over the object will pop up balloon help giving the name@host of the
#   object.
#
# NOTE: This _is_ just a demo  it is not intended to describe the way this UI
#       will actually operate.
#


package require Tk
set here [file dirname [info script]]
source [file join $here ringBufferObject.tcl]

package require properties
package require propertyEditor

canvas .c -width 512 -height 512
pack   .c
set tool [RingBufferObject %AUTO% -canvas .c]
$tool drawat 55 55

#  Turn the tool into a source of new objects:

$tool addtag creator
.c bind creator <Double-1> [list createNew %W $tool 55 55]

proc createNew {canvas original x y} {
    set newItem [$original clone]
    $newItem configure -canvas $canvas
    $newItem drawat $x $y
    $newItem moveby 10 10
    
    # Make object draggable:
    
    $newItem addtag $newItem
    $canvas bind $newItem <B1-Motion> [list move $newItem %x %y]
    
    # Make double click on new object edit properties:
    
    $canvas bind $newItem <Double-Button-1> [list editProperties $newItem]
    
    #  Set up the popup baloon for the ring buffer:
    
    balloon $canvas $newItem
    
}
proc move {item x y} {
    $item moveto $x $y
}

proc editProperties {item} {
    propertyDialog .rpropeditor \
        -title {Ring Buffer Properties} -wintitle {Ring Buffer Properties} \
        -proplist [$item getProperties]
    
    update idletasks
    
    .rpropeditor modal
    destroy .rpropeditor
    
}

namespace eval balloon {}

proc balloon {canvas ring} {
    $canvas bind $ring <Any-Enter> [list after 250 balloon::show $canvas $ring %X %Y]
    $canvas bind $ring <Any-Leave> [list destroy $canvas.balloon]
}
proc balloon::show {w ring x y} {
    
    # Construct the text into arg so we're compatible with what we stole from
    # http://wiki.tcl.tk/3060
    
    set l [$ring getProperties]
    set h [$l find host]
    set n [$l find name]
    
    set arg [$n cget -value]@[$h cget -value]
    if {$arg eq "@"} return;     # Not set.
    if {[string range $arg end end] eq "@"} {
        append arg "**not set ***"
    }
    if {[string range $arg 0 0] eq "@"} {
        set arg "**not set**$arg"
    }
    
    if {[eval winfo containing  [winfo pointerxy .]]!=$w} {return}
    set top $w.balloon
    catch {destroy $top}
    toplevel $top -bd 1 -bg black
    wm overrideredirect $top 1
 if {[string equal [tk windowingsystem] aqua]}  {
        ::tk::unsupported::MacWindowStyle style $top help none
    }   
    pack [message $top.txt -aspect 10000 -bg lightyellow \
            -font fixed -text $arg]
 
    wm geometry $top \
      [winfo reqwidth $top.txt]x[winfo reqheight $top.txt]+$x+$y
    raise $top
 }
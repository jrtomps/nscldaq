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
# @file propEditorDemo.tcl
# @brief Demonstrate/exercise property editor.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require propertyEditor
package require properties
#----------------------------------------------------------------------------
#  Editor by itelf:


#  Make a collection of properties:

propertylist l

property str -name {A string property}
l add str

property int -name {An integer property} -value 0 -validate [snit::integer i]
property intrange -name {integer [0-10]} -value 0 -validate [snit::integer ir -min 0 -max 10]
l add int
l add intrange


property float -name {A floating property} -value 0.0 -validate [snit::double  f]
property floatr -name {float [-1.0 1.0]}   -value 0.0 -validate [snit::double fr -min -1.0 -max 1.0]
l add float
l add floatr

toplevel .t1
propertyEditor .t1.e1 -proplist l -title {Editor Demo} -command [list done1 %W %E %L %P %V %N]
pack .t1.e1

set done 0
proc done1 {wid entry list prop value new} {
    $prop configure -value $new
}

bind .t1 <Destroy> [list incr done]

vwait done


propertyDialog .d -proplist l
set result [.d modal]
if {$result eq "Ok"} {
    puts "Properties changed:"
    l foreach property {
        set name [$property cget -name]
        set value [$property cget -value]
        
        puts "$name -> $value"
    }
}

exit
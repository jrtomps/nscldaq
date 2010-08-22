#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2009.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


#  Snit widget for managing thresholds:
#

package require Tk
package require snit

#
#   Snit widget layout:
#
# +-------------------------------------------------+
# | 32 spinboxes in 4 rows of 8 colums              |
# +-------------------------------------------------+
#
# Methods:
#   get 32_element_list
#   set 32_element_list
#
#
snit::widget thresholds {
    variable values -array {
    }

    constructor args {
	variable values
	#
        # The values array will default to all zeroes:

	for {set i 0} {$i < 32} {incr i} {
	    set values($i) 0
	}


	#  32 spinboxes have the stuff we need:
	# each bound to a variable in the values list.
	for {set i 0} {$i < 32} {incr i} {
	    label     $win.label_$i -text "Chan $i"
	    spinbox   $win.thr_$i  -from 0 -to 4095 -increment 1 \
		-textvariable [myvar values($i)]                 \
		-width 5
	}

	set i 0
	for {set row 0} {$row < 8} {incr row 2} {
	    for {set col 0} {$col < 8} {incr col} {
		set lrow $row
		set srow [expr $row + 1]
		grid $win.label_$i -row $lrow -column $col
		grid $win.thr_$i   -row $srow -column $col
		incr i
	    }
	}

      
    }
    method get {} {
	for {set i 0} {$i < 32} {incr i} {
	    lappend result [$win.thr_$i get]
	}
	return $result
    } 
    method set vals  {
	if {[llength $vals] != 32} {
	    error "set must have exactly a 32 channel list as an argument"
	}
	for {set i 0} {$i < 32} {incr i} {
	    $win.thr_$i set [lindex $vals $i]
	}
    }
}

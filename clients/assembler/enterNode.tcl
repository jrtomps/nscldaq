#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
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



package provide EnterNode 1.0
package require Tk
package require snit


# The EnterNode megawidget provides a means for
# entering the information about a node.  We need to prompt for the node name
# and the node id. We'll use a spinbox for  the node id and a plain old
# entry for the node name.
#
# OPTIONS
#  -node    - Name/IP of the node.
#  -id      - Event builder ID of the node.
#

snit::widget EnterNode {
    option -node
    option -id

    constructor args {
	label $win.nodelabel -text {Node: }
	entry $win.node      -textvariable ${selfns}::options(-node)

	label $win.idlabel   -text {Id: }
	spinbox $win.id      -from 0 -to 255 -wrap 1 -increment 1.0 \
	    -width 3         \
	    -validate all    \
	    -validatecommand [list string is integer -strict %P] \
	    -textvariable ${selfns}::options(-id)

	grid $win.nodelabel $win.node -sticky w
	grid $win.idlabel   $win.id   -sticky w
	$self configurelist $args


    }
}
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


# (C) Copyright Michigan State University 1937, All rights reserved 
#
#  GUI package to set stager policy.
#

# called when ok was pressed to accept the stage policy.
# Destroys the gui calls the callback.

package provide SelectStagePolicy 1.0
namespace eval SelectStagePolicy {
    variable StagePolicy
    proc StagePolicyOk {top callback texts policies} {
	variable StagePolicy
	set i 0
	$callback $StagePolicy
    }
    
    #
    #  top   - Widget under which the gui is constructed.
    #  texts - text labels of the radio buttons to construct on the 
    #          gui
    #  policies - Actual policy names
    #             in the GUI.
    proc SetStagePolicy {top callback texts policies} {
	set i 0
	variable  StagePolicy
	foreach policy $texts {
	    radiobutton $top.pol$i -variable SelectStagePolicy::StagePolicy \
		    -text [lindex $texts $i] \
		    -value [lindex $policies $i]
	    pack $top.pol$i -side top -anchor w
	    incr i
	}
	button $top.ok -text Ok \
		-command "SelectStagePolicy::StagePolicyOk $top $callback {$texts} {$policies}"
	pack $top.ok
	set StagePolicy [lindex $policies 0]
    }

    namespace export SetStagePolicy
    namespace export StagePolicy
}

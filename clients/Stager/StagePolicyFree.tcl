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


# (C) Copyright Michigan State University 1938, All rights reserved 
#
#   Provides a staging policy based on the total amount of free space on the
#   staging device.
#
package provide StagePolicyFree 1.0
package require ExpFileSystem
package require OS
namespace eval ExpStagePolicyFree {
    variable StageThreshold   0.5    ;# fraction free at which to stage.
    proc StageFraction {} {          ;# Free space fraction determination.
       set free [OS::FreeSpace [ExpFileSystem::GetStage]]
       set size [OS::DiskSpace  [ExpFileSystem::GetStage]]	
       set freefrac [expr 1.0 - double($free)/double($size)]
       return $freefrac
    }
    proc StageCriterion {} {          ;# Time to stage?
        variable StageThreshold
	return [expr [StageFraction] > $StageThreshold]
    }
    proc StageLevel {} {              ;# How close to staging?
	return [StageFraction]
    }
    proc SetThreshold {new} {	      ;# Set new threshold for staging. 
        variable StageThreshold
        set      StageThreshold $new
    }
    proc GetThreshold {} {               ;# Return current threshold.
	variable StageThreshold
	return $StageThreshold
    }
    proc GetLimits {} {
	return {0.0 1.0}
    }
    proc GetUnits {} {
	return "%"
    }
    #  Register as stage policy.
    proc Register {} {	
	ExpStagePolicy::RegisterStageCriterion ExpStagePolicyFree::StageCriterion
	ExpStagePolicy::RegisterStageLevel     ExpStagePolicyFree::StageLevel
	ExpStagePolicy::RegisterSetThreshold   ExpStagePolicyFree::SetThreshold
	ExpStagePolicy::RegisterGetThreshold   ExpStagePolicyFree::GetThreshold
	ExpStagePolicy::RegisterThresholdLimits ExpStagePolicyFree::GetLimits
	ExpStagePolicy::RegisterUnits          ExpStagePolicyFree::GetUnits
    }
    namespace export Register
}

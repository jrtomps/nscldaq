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
#  StagePolicy.tcl
#    Encapsulates stager policies.
#    Stager policy modules determine when staging should occur automatically.
#    This is done by providing stager policy procedures.  The stager
#    policy procedure templates are:
#       StageNow       - Returns bool True if time to stage.
#       StageCapacity  - Returns floating point from 0.0-1.0 indicating
#                        'how' full the staging area is relative to the
#                        criteria used by StageNow.  The idea is that this
#                        proc can produce a value which can be stuffed into
#                        a slider. e.g. to give a sense for when a stage will
#                        take place.
#
package provide ExpStagePolicy 1.0

namespace eval ExpStagePolicy {
    variable StageCriterion AlwaysStage
    variable StageLevel     AlwaysFull
    variable SetThreshold   Nullset
    variable GetThreshold   One
    variable StagerLimits   DefaultLimits
    variable StagerUnits    Percent

    #  The following procs constitue a null stage policy... if invoked,
    #  staging will always occur.

    proc AlwaysStage {} {
	return 1
    }
    proc AlwaysFull {} {
	return 1.0
    }
    proc Nullset {newval} {}
    proc One  {} {return 1}

    proc DefaultLimits {} {
	return {0.0 1.0}
    }

    proc Percent {} {
	return "%"
    }

    # The following are the 'member' functions of the package.

    # Registrations:
   
    # New staging criterion.

    proc RegisterStageCriterion {newStager} {   
        variable StageCriterion
	set StageCriterion  $newStager
    }
    #  New current usage level
    
    proc RegisterStageLevel {newLevel} {
        variable StageLevel
	set StageLevel $newLevel
    }
    #  New Threshold level

    proc RegisterSetThreshold {newset} {
	variable SetThreshold
	set SetThreshold $newset
    }
    #  Get current threshold

    proc RegisterGetThreshold {newget} {
	variable GetThreshold
	set GetThreshold $newget
    }
    #  Threshold range function:
    
    proc RegisterThresholdLimits {newthresh} {
	variable StagerLimits
	set StagerLimits $newthresh
    }
    #  Threshold units:

    proc RegisterUnits {newunits} {
	variable StagerUnits
	set StagerUnits $newunits
    }
    #
    #  Stager members - delegate to current policy.
    #

    #  Time to stage now ?

    proc StageNow {} {
        variable StageCriterion
	return [$StageCriterion] 
    }
    # Current usage:

    proc StageCapacity {} {
        variable StageLevel
	return [$StageLevel]
    }
    #  Set new threshold value.

    proc SetThreshold {newvalue} {
	variable       SetThreshold
	$SetThreshold  $newvalue
    }
    #  Get current threshold value.

    proc GetThreshold {} {
	variable GetThreshold
	return [$GetThreshold]
    }
    #  Get valid limits for thresholds:

    proc GetLimits {} {
	variable StagerLimits
	return [$StagerLimits]
    }
    #  Get units for limits

    proc GetUnits {} {
	variable StagerUnits
	return [$StagerUnits]
    }
    namespace export SetThreshold StageCapacity GetThreshold
    namespace export StageNow RegisterGetThreshold
    namespace export Register SetThreshold RegisterStageLevel 
    namespace export RegisterStageCriterion
}






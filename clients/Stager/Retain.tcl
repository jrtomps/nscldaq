#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#


# (C) Copyright Michigan State University 1937, All rights reserved
#
#   Retain.tcl:
#     Subsystem to manage runs which must be retained.
#

package provide RunRetension 2.0
package require StagerConfiguration

namespace eval ExpRunRetension {
    variable PendingRetensions
}

#  List the runs which are pending on the retension list.
#
proc ExpRunRetension::ListPending {} {
    return $ExpRunRetension::PendingRetensions
}
# Return a list of the runs which have already been retained.
#
proc ExpRunRetension::ListRetained {} {
    return [StagerConfiguration::getRetainList]
}
# ExpRunRetension::AddToPending run
#     Add a run to the list to retain on the next stage cycle.
# Parameters:
#   run    - Run number to add to the list.
# Returns:
#   New pending retension list.
# Errors:
#   NotInteger   - The run number is not an integer.
#   TooSmall     - Run number < 0
#   DuplicateRun - Run number already in retension list.
#
proc ExpRunRetension::AddToPending {run} {
    if {![string is integer -strict $run]} {
	error ExpRunRetension::NotInteger
    }
    if {$run < 0} {
	error ExpRunRetension::TooSmall
    }
    if {[ExpRunRetension::isPending $run]} {
	error ExpRunRetension::DuplicateRun
    }
    lappend ExpRunRetension::PendingRetensions $run
    return [ExpRunRetension::ListPending]
}
# ExpRunRetension::RemoveFromPending run
#    Remove a run from the list of pending retained runs.
# Parameters:
#    run  - Number of the run to remove.
# Errors:
#    NotPending  - Run is not on the pending retain list.
proc ExpRunRetension::RemoveFromPending {run} {
    set pindex [lsearch -exact $ExpRunRetension::PendingRetensions $run]
    if {$pindex < 0} {
	error ExpRunRetension::NotPending
    }
    set ExpRunRetension::PendingRetensions \
	    [lreplace $ExpRunRetension::PendingRetensions $pindex $pindex]
}

    #  Determine if run is in the pending retension list.
    #
proc ExpRunRetension::isPending {run} {
    return [expr [lsearch -exact $ExpRunRetension::PendingRetensions $run] != -1]
}
    #  Determine if a run has been retained or not.
    #
proc ExpRunRetension::isRetained {run} {
    set RetainedRuns [StagerConfiguration::getRetainList]
    return [expr [lsearch -exact $RetainedRuns $run] != -1]
}
# ExpRunRetension::MoveToRetained run
#
#     If 'run' is in the pending list, adds it to the retained list,
#   otherwise, throws an error.
# Parameters:
#    run  - Number of the run to mark as actually retained.
# Errors:
#    NotPending  - The indicated run was not on the retension pending
#                  list.
#
proc ExpRunRetension::MoveToRetained {run} {
#	puts "Moving $run to retained pending are $PendingRetensions"
    if {![isPending $run]} {
        error ExpRunRetension::NotPending
    }
    set pindex [lsearch -exact $ExpRunRetension::PendingRetensions $run]
    StagerConfiguration::appendRetainList [lindex $ExpRunRetension::PendingRetensions $pindex]
    set ExpRunRetension::PendingRetensions \
	[lreplace $ExpRunRetension::PendingRetensions $pindex $pindex]
}


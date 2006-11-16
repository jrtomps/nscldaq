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

package provide controller 1.0
package require ReadoutGUIPanel
package require ReadoutGui
package require ReadoutControl
package require ExpFileSystem


# sample controller for starting/stopping runs on behalf of the
# sequencer:
set ::runFlag 0
set ::runNumber 0;   #Initial run of the plan.

#
#  Return the event file in the complete dir for a run/seq
#  pair.
proc eventFile {run seq} {
   set evtfile [ExpFileSystem::WhereisRunFile $run $seq]

    # evt file is the name in the run dir .. need to locate
    # the complete dir and graft that on instead.
    set tail    [file tail $evtfile]
    set dir     [ExpFileSystem::WhereareCompleteEventFiles]
    set evtfile [file join $dir $tail]


    return $evtfile
}
#
#  Deletes data associated with a run
# run - The run number to delete data for.
#
proc deleteData run {
    puts -nonewline "Deleting data for run $run..."
    flush stdout
    set seq 0

    set evtfile [eventFile $run $seq]

    while {[file exists $evtfile] } {
        exec rm -f $evtfile
        incr seq
        set evtfile [eventFile $run $seq]
    }
    exec rm -rf [ExpFileSystem::WhereisRun $run]
    puts "done"

}

# This is our hook on the end of run.
# we increment the runFlag which in turn will
# bounce startPlannedRun out of vwait if it's there..
# allowing the next step of the run plan to be called.
#

proc OnEnd run {
    global runFlag

    incr runFlag
}


# Called when a run plan is being started.  We just remember
# the first run number.
#
proc startingPlan {} {
    global runNumber
    set runNumber  [ReadoutControl::GetRun]

}
#
#  Called when a run plan has been aborted.
#  If event recording was enabled, ask the user if they
#  want to trash the runs they took:
#
proc planAborted {} {
    global runNumber
    if {[ReadoutControl::isTapeOn]} {
        set lastRun [ReadoutControl::GetRun]
        set answer [tk_messageBox \
                        -title "Trash data?" \
                        -type yesno          \
                        -icon question       \
                        -message "Do you want to delete the data you've taken (This action cannot be undone)?"]
        if {$answer} {
            for {set run $runNumber} {$run < $lastRun} {incr run} {
                deleteData $run
            }
            ::ReadougGUIPanel::setRun $runNumber;    # Reset the run number to first run of aborted plan.
        }

    }
}


#
# Start a run from the run plan

#   button           - is the button widget controlling the run plan
#   table            - is the table widget controlling the run plan
#
proc startPlannedRun {button table} {
    global runFlag
    if {$ReadoutControl::State ne "NotRunning"} {
        tk_messageBox -icon error -title "Run active" \
            -message {Run must first be stopped!!}
        abortPlan $button $table
    } else {

        # Set the epics value:

        setRunConditions $channelNameList $channelValueList

        # Force the timed run on.

        ReadougGUIPanel::setTimed 1

        # Start the run.  vwait for the
        # run to end so that we can call
        # nextStep.. note that vwait will wait in the
        # event loop keeping the GUI alive.

        ReadoutGui::Begin
        vwait runFlag
        nextStep $button $table
    }

}
#
#  Called to abort an in-flight run plan.
#  Just end the run.
#
proc stopPlannedRun {} {
    ReadoutGui::End

}

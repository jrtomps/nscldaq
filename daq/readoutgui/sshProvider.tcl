#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
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

##
# @file sshProvider.tcl
# @brief Provide a data source on the end of an ssh pipeline.
#


package provide SSHPipe_Provider 1.0
package require ssh
package require Wait
package require ReadoutGUIPanel

# Establish the namespace in which the methods live:

namespace eval ::SSHPipe {
    variable parameterization [dict create \
        host [list {Host name}]             \
        path [list {Path to executable}]    \
        parameters [list {Command parameters}] \
    ]
    #
    #   Array indexed by source id whose contents
    #   are a dict containing:
    #    sshpid   - Process id of the ssh child process.
    #    inpipe   - input pipefd.
    #    outpipe  - output pipefd
    #    alive    - boolean true if alive.
    #    closing  - boolean true if the data source is being closed out.
    #    idle     - bolean true if the data source does not have an open run.
    #    line     - Input line buffer.
    #    parameterization - the parameterization dict.
    #
    variable activeProviders
}
#-----------------------------------------------------------------------------
#
# The procs below satisfy the public interface of a data source provider:
#

##
# parameters
#
#   Provides a description of the parameters the data source requires.
#
# @return dict of parameter descriptions
#
proc ::SSHPipe::parameters {} {
    return $::SSHPipe::parameterization
}
##
# start
#   Starts a data source.
#   * Start an ssh pipe according to the specificatioins in the paramter dict
#   * Fill in an entry in the activeProviders array that describes this
#     data source.
#
# @param params - The parameter dictionary with sourceid added.
#
# @throw - File is not executable on this system
# @note  - The requirement that a file be executable on this system may have to
#          be relaxed at some point.
#
proc ::SSHPipe::start params {
    
    # Extract the parameters from the dict:
    
    set sid [dict get $params sourcid]
    set host [dict get $params host]
    set program [dict get $params path]
    set params  [dict get $params parameters]
    
    # Assume the path is locally valid as well.
    
    if {![file executable $program]} {
        error "SSHPipe data source: Cannot start $program as it is not executable"
    }
    
    #  Start the ssh pipeline:
    
    set pipeinfo [::ssh::sshpid $host "$program $params"]
    
    # Set up our context entry in activeProviders:
    
    set ::SSHPipe::activeProviders($sid) [dict create \
        sshpid           [lindex $pipeinfo 0]     \
        inpipe           [lindex $pipeinfo 1]     \
        outpipe          [lindex $pipeinfo 2]    \
        alive            true                    \
        line             ""                     \
        closing          false                  \
        idle             true                   \
        parameterization $params                \
    ]
    
    #  Set up the listener for input from the program:
    
    fconfigure [lindex $pipeinfo 0] -blocking 0 -buffering line
    fileevent [lindex $pipeinfo 1] readable [list ::SSHPipe::_readable $sid]
}
##
# check
#
#   Check liveness of a source.
#
#  @param source   - Id of the source to check
#  @return boolean - True if source is alive, false otherwise.
#
proc ::SSHPipe::check source {
    return [dict get $::SSHPipe::activeProviders($source) alive]
}
##
# stop
#   Stop the data source
#   * If the run is active in the source end it.
#   * Issue the exit command.
#   * close the pipe so the EOF fires too
#
# @param source - id of the source to stop.
# @note  The actual clean up is done when the eof is detected on the
#        input pipe.  Here we just mark the dict indicating deletion is ok.
#
proc ::SSHPipe::stop source {
    
    # Attempt to end any non-halted run.
    
    
    if {[::SSHPipe::_notIdle $source]} {
        ::SSHPipe::_attemptEnd $source
    }
    ::SSHPipe::_send exit
    
    
    dict set ::SSHPipe::activeProviders($source) closing true
    
    ::SSHPipe::_close $source
}
##
# begin
#   Start a run.  Sets the title, the run number and
#   then starts the run.
#
# @param source - Id of the source.
# @param runNum - number of the run to start.
# @param title  - Title of the run.
#
# @throw - we are no longer connected to the data source.
# @throw - we are not idle.
#
proc ::SSHPipe::begin {source runNum title} {
    
    ::SSHPipe::_errorIfDead $source
    set sourceInfo ::SSHPipe::activeProviders($source)
 
    if {[::SSHPipe::_notIdle $source]} {
        set host [dict get $sourceInfo host]
        set path [dict get $sourceInfo path]
        error "A run is already active in $path@$host"
    }
    
    # Set the run metadata title is in [list] to quote it properly.:
    
    ::SSHPipe::_send $source "set run $runNum"
    ::SSHPipe::_send $source "set title [list $title]"
    
    # Start the run and update our state:
    
    ::SSHPipe::_send $source begin
    
    dict set sourceInfo idle false
    set ::SSHPipe::activeProviders($source) $sourceInfo
}
##
# pause
#    Attempt to pause the run.
#
# @param source - Id of the source to pause.
#
# @throw  If we are no longer connected to the data source.
# @throw  If we are idle since we can only pause if we are not idle.
# @note   since we are not maintaining full state we must take the caller's
#         word that we are not already paused.
#
proc ::SSHPipe::pause source {
    ::SSHPipe::_errorIfDead $source
 
    if {![::SSHPipe::_notIdle $source]} {
        set sourceInfo ::SSHPipe::activeProviders($source)
        set host [dict get $sourceInfo host]
        set path [dict get $sourceInfo path]
        error "A run is not active in $path@$host so no pause is possible."
    }    
    ::SSHPipe::_send $source pause
}
##
# resume
#  Attempt to resume a paused run
#
# @param source - id of source to resume.
#
# @throw  If we are no longer connected to the data source.
# @throw  If we are idle since we can only resume if we are not idle.
# @note   since we are not maintaining full state we must take the caller's
#         word that we are not already paused.
#
proc ::SSHPipe::resume source {
    if {![::SSHPipe::_notIdle $source]} {
        set sourceInfo ::SSHPipe::activeProviders($source)
        set host [dict get $sourceInfo host]
        set path [dict get $sourceInfo path]
        error "A run is not active in $path@$host so no resume is possible."
    }    
    ::SSHPipe::_send $source resume    
}
##
# end
#  Attempt to end a run.
#
# @param source - id of source to resume.
#
# @throw  If we are no longer connected to the data source.
# @throw  If we are idle since we can only resume if we are not idle.
# @note   since we are not maintaining full state we must take the caller's
#         word that we are not already paused.
#
proc ::SSHPipe::end source {
    if {![::SSHPipe::_notIdle $source]} {
        set sourceInfo ::SSHPipe::activeProviders($source)
        set host [dict get $sourceInfo host]
        set path [dict get $sourceInfo path]
        error "A run is not active in $path@$host so no end is possible."
    }    
    ::SSHPipe::_send $source end 
}
##
# capabilities
#    Returns a dict with the capabilities of SSHPipe data sources.
#
#  This fills in the following:
#  *   canpause        - true
#  *   runsHaveTitles  - true
#  *   runsHaveNumbers - true
#
proc ::SSHPipe::capabilities {} {
    return [dict create canpause true runsHaveTitles true runsHaveNumbers true]
}
#-------------------------------------------------------------------------------
# Private utilities:
#

##
# _readable
#
#   Called when the input pipe from a file data source is ready to be read.
#   * If there's an EOF on the input,
#      - cancel us
#      - mark the source dead in the activeProviders array.
#      - Flush any data in the line buffer.
#      - Log source exit to the GUI.
#      - Wait on the ssh pid.
#   * If no EOF
#      - Read what we can from the inpipe appending it to the line buffer
#      - If the line buffer has endlines, output them as log entries.
#
# @param source  - Id of the source that just fired.
#
proc ::SSHPipe::_readable source {
    set sourceInfo $::SSHPipe::activeProviders($source)
    set fd [dict get $sourceInfo inpipe]
    
    if {[eof $fd]} {
        ::SSHPipe::_sourceExited source
    } else {
        ::SSHPipe::_readInput
    }
}
##
# _sourceExited
#
#   Called to process EOF on a data source input pipe.  This indicates the
#   source exited.  See _readable for the full details of our actions.
#
# @param source  - Source id of the source that exited.
#
proc ::SSHPipe::_sourceExited source {
    set sourceInfo $::SSHPipe::activeProviders($source)
    filevent [dict get $sourceInfo inpipe] ""
    
    dict set sourceInfo alive false
    
    set input [dict get $sourceInfo line]
    set host  [dict get $sourceInfo parameterization host]
    set path [dict get $sourceInfo parameterization path]
    
    if {$input ne  ""} {
        ReadoutGUIPPanel::Log SSHPipe@$host Input $input
        dict set sourceInfo line ""
    }
    ReadoutGUIPPanel::Log SSHPipe@$host Exiting  \
        "Source $path@$host exited"
    
    Wait -pid [dict get $sourceInfo sshpid]
    
    close [dict get $sourceInfo inpipe]
    
    if {[dict get $sourceInfo exiting]} {
        array unset ::SSHPipe::activeProvider $sourcde
    } else {
        set ::SSHPipe::activeProviders($source) $sourceInfo
    }
    
    
}
##
# _readInput
#
#  Read input from a data source pipe.
#  If at least one complete line has been received output all complete
#  lines as log messages to the console.
#
# @param source - Id of the source to read from.
#
proc ::SSHPipe::_readInput source {
    
    set sourceInfo $::SSHPipe::activeProviders($source)
    set host [dict get $sourceInfo parameterization host]
    set path [dict get $sourceInfo parameterization path]
    
    append data [dict get $sourceInfo line ] [read [dict get $sourceInfo inpipe]]

    set line [split $data "\n"]
    while {[llength $line] > 1} {
        set aline "[lindex $line 0]\n"
        ReadoutGUIPanel::Log SSHPipe@$host Input $aline
        set line  [lreplace $line 0 0]
    }
    
    dict set sourceInfo line $line
    set activeProviders($source) $sourceInfo
                                              
}

##
# _notIdle
#
# @param source   - Id of the source to check.
# @return boolean - true if the source is not in the idle state else false.
#
proc ::SSHPipe::_notIdle source {
    return [expr {![dict get $::SSHPipe::activeProviders($source) idle]}]
}
##
# _attemptEnd
#
#   Attempt to end a run that is active.
#
# @param source - id of source to end.
#
proc ::SSHPipe::_attemptEnd source {
    ::SSHPipe::_send source end
    dict set ::SSHPipe::activeProviders($source) idle true
}
##
# _send
#
#  Send a message to a sourcde
#
# @param source -id of source to send.
# @param msg    - The message to send.
#
proc ::SSHPipe::_send {source msg} {
    set fd [dict get $::SSHPipe::activeProviders($source) outpipe] 
    puts  $fd $msg
    flush $fd
}
##
# _close
#    Close the output data file
#
# @param source - source id.
#
proc ::SSHPipe::_close source {
    close [dict get $::SSHPipe::activeProviders($source) outpipe]
}
##
# _errorIfDead
#
#  @param source - Source to check
#  @throw If we are no longer connected to the source or it is shutting down:
#
proc ::SSHPipe::_errorIfDead source {
    set sourceInfo ::SSHPipe::activeProviders($source)
    set host       [dict get $sourceInfo host]
    set program    [dict get $sourceInfo path]
    
    if {(![::SSHPipe::check $source]) || [dict get $sourceInfo closing]} {
        error "SSHPipe source $program@$host is closing or no longer running"
    }
     
}
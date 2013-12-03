#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file eventLogBundle.tcl
# @brief Callaback bundle that encapsulates event logging.
# @author Ron Fox <fox@nscl.msu.edu>

package provide eventLogBundle 1.0
package require DAQParameters
package require RunstateMachine

package require ExpFileSystem
package require ReadoutGUIPanel
package require Diagnostics
package require ui



##
# @class eventLogBundle
#
#  Provides a callback bundle for the run state machine singleton that manages
#  the event logger.  The main stuff that gets done (all of these only if
#  event logging is turned on):
#
#  * Halted -> Active (leave): Starts the event logger and waits for the logger
#                              start file.
#  * {Paused,Active} -> Halted (leave): Waits for the event logger done files.
#                                       and finalizes the event and ancillary data.
#  * {Paused,Active} -> Halted (enter): Cleans up the logger start and logger
#                                       done files.
#
#  The EventLogger DAQ configuration parameter is used to determine which
#  event logger is started (DaqParameters).

#------------------------------------------------------------------------------
#  Establish the namespace and the namespace variables:

namespace eval ::EventLog {
    
    ##
    # @var loggerPid       - When the event logger is active, this is its pid.
    # @var startupTimeout  - When starting the event logger, this is how long
    #                        to wait in seconds for the startup file to appear.
    # @var shutdownTimeout - When the run ends this is how long to wait in
    #                        seconds for the end file to appear.
    # @var filePollInterval- How long in ms between polls for a file to appear.
    # @varl protectFiles   - If true the finalized event files are protected
    #                        against accidental removal. Used in testing.
    #
    variable loggerPid         -1
    variable startupTimeout    10
    variable shutdownTimeout   20
    variable filePollInterval 100
    variable protectFiles       1
    
    ##
    # @var loggerfd       - File descriptor open on the event logger.
    # @var expectingExit  - True if expecting the event logger to exit.
    #                       If not true and we detect an event logger exit we're
    #                       going to kick up a fuss.
    #
    
    variable loggerfd     [list]
    variable expectingExit 0
    
    
    
    # Export the bundle interface methods
    
    namespace export attach enter leave
}

#------------------------------------------------------------------------------
#
# Data methods:
#

##
# ::EventLog::getPid
#
# @return integer - The PID of the event logger if is active or -1 if it is not.
#
proc ::EventLog::getPid {} {
    return $::EventLog::loggerPid
}
##
# ::EventLog::setStartupTimeout
#
#  Set a new value in seconds for the startup time out.  This is the number
#  of seconds the package will wait for the eventLogger to create its startup
#  file (.started) before declaring a timeout.
#
# @param newTimeout - the new timeout value in seconds
#
proc ::EventLog::setStartupTimeout {newTimeout} {
    set EventLog::startupTimeout $newTimeout
}
##
# ::EventLog::getStartupTimeout
#
# @return int - The current event log start timeout.  See ::EventLog::setStaruptTimeout
#               for a description of that value.
#
proc ::EventLog::getStartupTimeout {} {
    return $::EventLog::startupTimeout
}
##
# ::EventLog::setShutdownTimeout
#
#   Sets the current shutdown timeout to a new value.  See ::EventLog::getShutdownTimeout
#   for more information about just what this is.
#
# @param newTimeout - new value of the timeout.
# #
proc ::EventLog::setShutdownTimeout {newTimeout} {
    set ::EventLog::shutdownTimeout $newTimeout
}

##
# ::EventLog::getShutdownTimeout
#
#  @return int Number of seconds to wait for the event loggers .exited
#              file to appear.
#
proc ::EventLog::getShutdownTimeout {} {
    return $::EventLog::shutdownTimeout
}
##
# ::EventLog::setFilePollInterval
#
#  Set a new value for the file poll interval.  This is how often a check is done
#  for the appearance of an expected file ( e.g. .exiting).
#
# @param newInterval - the new poll interval in milliseonds.
#
proc ::EventLog::setFilePollInterval {newInterval} {
    set ::EventLog::filePollInterval $newInterval
}
##
# ::EventLog::getFilePollInterval
#
# @return int - The current file poll interval.  See ::EventLog::setFilePollInterval
#               for more about what this means.
#
proc ::EventLog::getFilePollInterval {} {
    return $::EventLog::filePollInterval
}
proc ::EventLog::getLoggerPath {} {}

#------------------------------------------------------------------------------
# Utility methods

##
# ::EventLog::_cdCurrent
#
#  Change directory to the place current event files should be written.
#
proc ::EventLog::_cdCurrent {} {
    cd [::ExpFileSystem::getCurrentRunDir]
}
##
# ::EventLog::_startLogger
#  Start the event logger and set it's pid in the loggerPid variable.
#  @note The event logger is started as a pipeline open on an fd for read.
#        We will establish a file readable handler for the event logger so that
#        can relay input to the output log windows and throw up an error dialog
#        if the fd closes unexpectedly.
#
proc ::EventLog::_startLogger {} {
    set ::EventLog::loggerFd [open "| [DAQParameters::getEventLogger]" r]
    set ::EventLog::loggerPid [pid $::EventLog::loggerFd]
    set fd [lindex $::EventLog::loggerFd end]
    fconfigure $fd -buffering line
    fileevent $fd readable ::EventLog::_handleInput
}
##
# ::EventLog::_handleInput
#    - If input comes in, read it and log it to the console window.
#    - If there's an EOF on input and it's unexpected, throw up an error
#      that the event logger looks like it unexpectedly exited.
#    - Either way on EOF, mark the logger exited and close the File descriptor.
#      setting the variable to [list]
#
proc ::EventLog::_handleInput {} {
    set fd [lindex $::EventLog::loggerFd end]
    if {[eof $fd]} {
        if {!$::EventLog::expectingExit} {
            ::ReadoutGUIPanel::Log EventLogManager *ERROR* {Unexpected event log error!}
            ::Diagnostics::Error {The event logger exited unexpectedly!!}
        }
        close $fd
        set ::EventLog::loggerFd [list]
        set ::EventLog::loggerPid -1
    } else {
        set line [gets $fd]
        ::ReadoutGUIPanel::Log EventLogManager input $line
    }
}
##
# Wait for the appearance of a file.  The event logger uses . files to synchronize
# with us about the occurence of various events.
#
# @param name - Name of the file to wait for.
# @param waitTimeout - Number of seconds to wait for the file to appear.
# @param pollInterval - Number of ms between checks for the file.
#
# @return bool - true if the file appeared prior to the timeout. false if not.
#
proc ::EventLog::_waitForFile {name waitTimeout pollInterval} {
    set waitTimeoutMs [expr {$waitTimeout * 1000}];   # Wait timeout in milliseconds
    
    while {$waitTimeoutMs > 0} {
        if {[file exists $name]} {
            return 1
        }
        incr waitTimeoutMs -$pollInterval
        after $pollInterval
    }
    return 0
}
##
#  _finalizeRun
#
#  Finalizes a run   This means:
#  * Creating a new run directory.
#  * mv-ing the event files in to this new run directory.
#  * Making symlinks for each event file segment in complete directory (event view).
#  * Copying the metadata into the new run directory.
#
#
#
proc ::EventLog::_finalizeRun {} {
    set srcdir [::ExpFileSystem::getCurrentRunDir]
    set completeDir [::ExpFileSystem::getCompleteEventfileDir]
    set run [ReadoutGUIPanel::getRun]
    set destDir [::ExpFileSystem::getRunDir $run]
    
    # Make the run directory:
    # and mv the event files into it... remembering the destination names.
    
    file attributes \
        [file normalize [file join $destDir ..]] -permissions 0770;   # Let me write here.
    file mkdir $destDir
    set  fileBaseName [::ExpFileSystem::genEventfileBasename $run]
    set  eventFiles [glob -nocomplain [file join $srcdir ${fileBaseName}*.evt]]
    set  mvdNames [list]
    foreach eventFile $eventFiles {
        set destFile [file join $destDir [file tail $eventFile]]
        file rename -force $eventFile $destFile
        lappend mvdNames $destFile
    }
    #  Make links in the complete directory for all mvdNames:
    
    foreach file $mvdNames {
        set targetLink [file join $completeDir [file tail $file]]
        catch {exec ln -s $file $targetLink}
    }
    #  Now what's left gets recursively/link-followed copied to the destDir
    #  using tar.
    
    set tarcmd "(cd $srcdir; tar chf - .) | (cd $destDir tar xpf -)"
    exec sh << $tarcmd
    
    # If required, protect the files:
    #   - The destDir is set to 0550
    #   - The parent dir is set to 0550.
    #   - A chmod -R is done to set the contents to 0x550 as well.
    
    if {$::EventLog::protectFiles} {
        exec sh << "chmod -R 0550 [file join $destDir *]"
        file attributes $destDir -permissions 0550 
        file attributes [file join $destDir ..] -permissions 0550 
    }
    
}

#------------------------------------------------------------------------------
# Actions:

##
# ::EventLog::runStarting
#
#   Called when the run is about to start:
#   * Ensure we are in the correct cwd for the event logger (from the daq filesystem).
#   * Start the event logger.
#   * Wait for the .started file to appear.
#
proc ::EventLog::runStarting {} {
    
    if {[::ReadoutGUIPanel::recordData]} {
        puts "Recording data!!"
        ::EventLog::_cdCurrent
        ::EventLog::_startLogger
        ::EventLog::_waitForFile .started $::EventLog::startupTimeout \
                $::EventLog::filePollInterval
        set ::EventLog::expectingExit 0
    }
}
##
# Called when the run is ending.  We're only going to do something if the
# event logger's pid is not -1.  In that case:
# * Set that we expect an exit.
# * Wait for the .exited file
# * Set the PID -> -1
# * Finalize the run.
#
#  @note - We let the file readable handler handle closing the fd.
#
proc ::EventLog::runEnding {} {
    
    # ne is used below because the logger could be a pipeline in which case
    # ::EventLog::loggerPid will be a list of pids which freaks out ==.
    
    puts "Logger pid: $::EventLog::loggerPid"
    if {$::EventLog::loggerPid ne -1} {
        set ::EventLog::expectingExit 1
        ::EventLog::_waitForFile .exited $::EventLog::shutdownTimeout \
            $::EventLog::filePollInterval
        set ::EventLog::loggerPid -1
        ::EventLog::_finalizeRun
        
    }
   
}


#-------------------------------------------------------------------------------
#
#  Bundle methods:

##
# ::EventLog::attach
#
#    Called when the bundle is attached to the state machine
#  
# @param state - Current state.
#
proc ::EventLog::attach {state} {
    
}
##
# ::EventLog::enter
#
#   Called when the run enters a new state.  We care about transitions:
#   {Paused, Active} -> Halted.
#
proc ::EventLog::enter {from to} {
    if {($from in [list Active Paused]) && ($to eq "Halted")} {
        ::EventLog::runEnding
    }
}
##
# ::EventLog::leave
#
#   Called when the run leaves a state.
#   If the state is Halted->Active, we start the event logger
#
# @param from - State that we are leaving
# @param to   - State we will enter.
#
proc ::EventLog::leave {from to} {
    if {($from eq "Halted") && ($to eq "Active")} {
        ::EventLog::runStarting
    }
}

#-------------------------------------------------------------------------------
#
# Bundle registration
#


##
# ::EventLog::register
#
#   Register the event logger package as a callback bundle with the run
#   state machine singleton:
#
proc ::EventLog::register {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle EventLog
    $sm destroy
}
##
#  ::EventLog::unregister
#
#   Unregisters the event logger package from the Run state machine singleton.
#   this is really only supplied for testing purposes (maybe). But
#   could potentially be used for special applications.
#
proc ::EventLog::unregister {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm removeCalloutBundle EventLog
    $sm destroy
    
}

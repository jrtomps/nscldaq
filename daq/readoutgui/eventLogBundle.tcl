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
package require Tk
package require DAQParameters
package require RunstateMachine

package require ExpFileSystem
package require ReadoutGUIPanel
package require Diagnostics
package require ui
package require snit

package require ring

package require portAllocator
package require DataSourceUI



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
    variable shutdownTimeout   600
    variable filePollInterval 100
    variable protectFiles       1

    # Installation root:
    #  Assumes we're in a subdirectory of TclLibs relative to the installation
    #  root.
    #
    variable DAQRoot  [file normalize \
        [file join [file dirname [info script]] .. ..]]
    
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
    ReadoutGUIPanel::isRecording
    set logger [DAQParameters::getEventLogger]
    set ring   [DAQParameters::getEventLoggerRing]
    set ::EventLog::loggerFd \
        [open "| $logger --source $ring  --oneshot 2>&1" r]
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
        # Need to close off the fd before the pop up shows as that will
        # re-enter the event loop
        
        fileevent $fd readable [list]
        catch {close $fd}

        # Log to the output window and pop up and error.

        if {!$::EventLog::expectingExit} {
            ::ReadoutGUIPanel::Log EventLogManager error {Unexpected event log error!}
            ::Diagnostics::Error {The event logger exited unexpectedly!!}
        }
        set ::EventLog::loggerFd [list]
        set ::EventLog::loggerPid -1
    } else {
        set line [gets $fd]
        ::ReadoutGUIPanel::Log EventLogManager output $line
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
 
        # Ensure there are no stale synch files:

        file delete -force .exiting
        file delete -force .started
        
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
        puts "Waiting for .exit"
        ::EventLog::_waitForFile .exited $::EventLog::shutdownTimeout \
            $::EventLog::filePollInterval
        puts "Got .exited or timed out."
        set ::EventLog::loggerPid -1
        ::EventLog::_finalizeRun
        file delete -force .exited;   # So it's not there next time!!
        
        # Incremnt the run number:
        
        ReadoutGUIPanel::incrRun
        ReadoutGUIPanel::normalColors
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
#----------------------------------------------------------------------------
#
#   The code in this section provides user interface code to
#   prompt for the event logger's parameters and stor them so that
#   the next call of ::EventLog::_startLogger will use those parameters.
#   The supported parameters are:
#   *  EventLogger - the event logger program to use.  This program must
#                    support or at least ignore the -oneshot switch.
#   *  EventLoggerRing - The name of the ring buffer from which the
#                    event logger gets data.   This should be a URI
#                    not a local ring name.  However, if it is not a URI,
#                    tcp://localhost/ is prepended to the ring name.
#
#

##
# @class EventLog::RingBrowser
#
#  A ring browser window.  Allows users to select a ring from a set of ring
#  in the form ring@hostname
#
# LAYOUT:
#  +----------------------------+
#  |  +-------------------+-+   |
#  |  | ring listbox      |s|   |
#              ...
#  |  +-------------------+-+   |
#  +----------------------------+
# OPTIONS:
#      -rings  - Information about the rings as passed in from the ring master.
#
# METHODS:
#   getSelected - Returns the selected ring in the form ring@hostname
#
snit::widgetadaptor EventLog::RingBrowser {
    option -rings -default [list] -configuremethod _stockListbox
    
    ##
    # constructor
    #
    #    Constructs the user interface and runs the configurlist method
    #    which may (or may not) stock the listbox.
    #
    # @param args - the option/value pairs.
    #
    constructor args {
        installhull using ttk::frame
        
        listbox $win.list -selectmode single -yscrollcommand [list $win.vsb set]
        ttk::scrollbar $win.vsb -orient vertical -command [list $win.list yview]
        
        grid $win.list $win.vsb -sticky nsew
        
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    # Public methods:
    
    ##
    # getSelected
    #
    #  @return if there's an active selected ring, returns it other wise
    #          returns an empty string.
    #
    method getSelected {} {
        return [$win.list get [$win.list index active]]
    }
    #-------------------------------------------------------------------------
    # Configuration methods
    #
    
    ##
    # _stockListbox
    #
    #   Update the set of rings that are shown in the list box.
    #
    # @param optname - option name (-ring)
    # @param optvalue - List of ring usage statistics from the ringmaster.
    #                   The keypoints are:
    #                  * The first element of each list item is the ring name.
    #                  * Proxy rings are of the form host.ring
    #
    method _stockListbox {optname value} {
        set options($optname) $value
        
        $win.list delete 0 end
        
        foreach ringUsage $value {
            set name [lindex $ringUsage 0]
            set nameList [split $name .]
            if {[llength $nameList] == 1} {
                set ringName $nameList@localhost
            } else {
                #
                #  This code allows for rings remote rings with .'s in them
                #  (though not local rings).
                #
                set host [lindex $nameList 0]
                set ring [join [lrange $nameList 1 end] .]
                set ringName $ring@$host
            }
            $win.list insert end $ringName
        }
    }
    
    
}

##
# @class EventLog::ParameterPrompter
#
#     Provides a dialog form which allows users to override the current set of
#     event logger parameters.
#     By dialog form we mean a form that can be attached to a DialogWrapper.
# LAYOUT:
#     +-------------------------------------------------+
#     | Event Log program <current value> [Browse]      |
#     | Data Source       <current value> [Known Rings] |
#     +-------------------------------------------------+
#
# Key:  Stuff that's not quoted in something are labels, Stuff quoted in
#       <> are entries, and stuff quoted in [] are buttons.
#
#  @note the [Local Rings] button provides a list of rings to choose from
#        The list includes the local rings and the proxy rings that have already
#        been defined.
#        
#
#  OPTIONS:
#      -logger  - Value of the event logger.
#      -ring    - URI that points to the ring buffer.
#

snit::widgetadaptor EventLog::ParameterPrompter {
    option -logger 
    option -ring   
    
    
    ##
    # constructor
    #   Build and stock the useer interface.  We're going to bind the
    #   entry values to the option variables so  there's no need to
    #   build -configuremethod methods to track those.
    #
    # @param args - Configuration option/values.
    #
    # @note - The defaults for the parameters are gotten from the
    #         current configuration so typically the dialog is self
    #         configured.
    #
    constructor args {
        installhull using ttk::frame
        
        # Can't seem to do this when tcl is compiling to byte code.
        
        set options(-logger) [::DAQParameters::getEventLogger]
        set options(-ring)   [::DAQParameters::getEventLoggerRing]
        
        $self configurelist $args
        
        ttk::label  $win.loglabel -text {Event log program}
        ttk::entry  $win.logger       \
            -textvariable [myvar options(-logger)] -width 40
        ttk::button $win.browselogger \
            -text {Browse...} -command [mymethod _browseLogger]
        
        ttk::label $win.datasourcelabel -text {Data Source Ring URI}
        ttk::entry $win.datasource    \
            -textvariable [myvar options(-ring)] -width 40
        ttk::button $win.knownrings    \
            -text {Known Rings...} -command [mymethod _browseRings]
        
        grid $win.loglabel $win.logger $win.browselogger -sticky w
        grid $win.datasourcelabel $win.datasource $win.knownrings -sticky w
        
    }
    #------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _browseLogger
    #
    # Browse the NSCLDAQ installation space for event logger programs.
    # Allow the user to select one.  This is just a file browser window where:
    #  * The initial directory is the bin directory of the installation tree
    #    in which we've been installed.
    #  * The default filetype is ""
    #  * File types allowed are "", .sh .bash or all files.
    #
    method _browseLogger {} {
        set file [tk_getOpenFile  \
            -initialdir [file join $::EventLog::DAQRoot bin]  \
            -parent $win -title "Choose event logger" \
            -filetypes [list \
                { {All Files}     *}                          \
               { {Shell scripts} {.sh}       }              \
                { {Bash scripts}  {.bash}     }              \
            ]]
        if {$file ne ""} {
            set options(-logger) $file
        }
    }
    
    ##
    # _browseRing
    #  Pops up a dialog that provides a list of the ring buffers
    #  and the hosts they belong to and allows the user to select from
    #  a ring from them...or not.
    #  If a ring was selected, it populates the options(-ring) entry.
    
    method _browseRings  {} {
        toplevel $win.ringbrowser
        set dlg [DialogWrapper $win.ringbrowser.dialog]
        $dlg configure \
            -form [EventLog::RingBrowser [$dlg controlarea].f \
                -rings [getRingUsage]]
        pack $dlg
        set action [$win.ringbrowser.dialog modal]

        
        if {$action eq "Ok"} {
            set ring  [[$dlg controlarea].f getSelected]
            #
            #  User may click Ok without selecting a ring!
            #
            if {$ring ne ""} {
                set options(-ring) [ringToUri $ring]
            }
        }
        destroy $win.ringbrowser
        
    }
    
    #--------------------------------------------------------------------------
    #   Procs
    #
    
    ## ringToUri
    #
    #  Convert a ring name of the form name@host to a valid ring URI.
    #
    # @param ringName - The ring name in the form ring@host
    #
    # @return string  - The URI for the ring.
    #
    proc ringToUri ringName {
        set ringInfo [split $ringName @];   # list of {ring hostname}
        return tcp://[lindex $ringInfo 1]/[lindex $ringInfo 0]
    }
    
    ##
    # getRingUsage
    #
    #  Get the rings used/known by a ringmaster.
    #
    # @param host - defaults to localhost Host for which to ask for this
    #               information
    #
    # @return list - Returns the list from the LIST command to that ringmaster.
    #
    proc getRingUsage {{host localhost}} {
        portAllocator create manager -hostname $host
        set ports [manager listPorts]
        manager destroy
    
        set port -1
        foreach item $ports {
            set port [lindex $item 0]
            set app  [lindex $item 1]
            if {$app eq "RingMaster"} {
                set port $port
                break
            }
        }
        if {$port == -1} {
            error "No RingMaster server  on $host"
        }
    
        set sock [socket $host $port]
        fconfigure $sock -buffering line
        puts $sock LIST
        gets $sock OK
        if {$OK ne "OK"} {
            error "Expected OK from Ring master but got $OK"
        }
        gets $sock info
        close $sock
        return $info
    }

}
##
# EventLog::promptParameters
#
#   Proc that instantiatesthe parameter prompter and, if OK was fetched,
#   sets the parameters in the configuration.
#
proc EventLog::promptParameters {} {
    toplevel .eventlogsettings
    set dlg [DialogWrapper  .eventlogsettings.dialog]
    set ctl [$dlg controlarea]
    $dlg configure \
        -form [EventLog::ParameterPrompter $ctl.f]
    pack $dlg
    set action [$dlg modal]
    
    if {$action eq "Ok"} {
        Configuration::Set EventLogger [$ctl.f cget -logger]
        Configuration::Set EventLoggerRing [$ctl.f cget -ring]
    }
    destroy .eventlogsettings
}

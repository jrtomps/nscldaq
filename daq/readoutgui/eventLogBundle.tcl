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
package provide Experiment     2.0;   # For compatibility with event builder.


package require Tk
package require DAQParameters
package require RunstateMachine
package require DataSourceManager

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
    variable shutdownTimeout   30
    variable filePollInterval 100
    variable protectFiles       1
    
    #  For our status line:
    
    variable statusBarManager ""
    variable statusbar         ""
    variable statusUpdateId    -1;     # After Id used to poll for status updates.

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

# For compatibility with 10.x event builders...provide shutdownTimeout but
# trace changes to update ::EventLog::shutdownTimeout

namespace eval ::Experiment {
    variable fileWaitTimeout $::EventLog::shutdownTimeout
}


proc ::EventLog::_updateShutdownTimeout {name1 name2 op} {
    set ::EventLog::shutdownTimeout $::Experiment::fileWaitTimeout
}
trace add variable ::Experiment::fileWaitTimeout write ::EventLog::_updateShutdownTimeout 


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
# ::EventLog::_computeLoggerSwitches
#
# @return the command line options the logger should use:
#
proc ::EventLog::_computeLoggerSwitches {} {
    
    # Base switches:
    
    set ring   [DAQParameters::getEventLoggerRing] 

    # Compatibility with 10.x:

    if {[info proc ::Experiment::spectrodaqURL] ne ""} {
	set ring [::Experiment::spectrodaqURL localhost]
    }

    set switches "--source=$ring "
    
    # If requested, use the --number-of-sources switch:
    
    if {[DAQParameters::getUseNsrcsFlag]} {
        set sm [DataSourcemanagerSingleton %AUTO%]
        set mySources [llength [$sm sources]]
        set adtlSources [DAQParameters::getAdditionalSourceCount]
        set totsrc [expr {$mySources + $adtlSources}]
        $sm destroy
        append switches " --number-of-sources=$totsrc"
    }
    # If requested get the run number from the GUI and force it:
    
    if {[DAQParameters::getRunNumberOverrideFlag]} {
        set run [::ReadoutGUIPanel::getRun]
        append switches " --run=$run"
    }
    
    append switches " --oneshot"
    return $switches
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
    set switches [::EventLog::_computeLoggerSwitches]
    set ::EventLog::loggerFd \
        [open "| $logger $switches" r]
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
        catch {close $fd} msg

        # Log to the output window and pop up and error.

        if {!$::EventLog::expectingExit} {
            ::ReadoutGUIPanel::Log EventLogManager error "Unexpected event log error! $msg"
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
	update;			# keep the event loop semi-live. TODO: Deactivate buttons.
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
    
    set tarcmd "(cd $srcdir; tar chf - .) | (cd $destDir; tar xpf -)"
    exec sh << $tarcmd
    
    # If required, protect the files:
    #   - The destDir is set to 0550
    #   - The parent dir is set to 0550.
    #   - A chmod -R is done to set the contents to 0x550 as well.
    
    if {$::EventLog::protectFiles} {
        set files [glob -directory $destDir -types {f d} *]
        puts $files
        exec sh << "chmod -R 0550 $files"
        file attributes $destDir -permissions 0550 
        file attributes [file join $destDir ..] -permissions 0550 
    }
    
}
##
# ::EventLog::_getSegmentInfo
#
#   Looks at the current event file areas to see how many segments there are
#   and how much total space that consumes.
#
# @return list first element is the number of event segments found, the second
#              the total Mbytes of storage used.
#
proc ::EventLog::_getSegmentInfo {} {
    set eventDir [ExpFileSystem::getCurrentRunDir]
    set run [::ReadoutGUIPanel::getRun]
    
    set filename [format "run-%04d*.evt" $run]
    set filepat [file join $eventDir $filename]

    set    segments [glob -nocomplain $filepat]
    set    size     0.0
    set    nsegments [llength $segments]
    foreach segment $segments {
	if {![catch {file size $segment} segsize]} {
	    set size [expr {$size + $segsize/1024.0}]
	}
    }
    set size [format %.2f [expr {$size/1024.0}]]
    
    
    return [list $nsegments $size]
}

##
# ::EventLog::_setStatusLine
#
#   Manage the data in the event logger status line:
#
# @param repeatInterval - ms after which to schedule an update.
#
proc ::EventLog::_setStatusLine repeatInterval {
    set run [::ReadoutGUIPanel::getRun]
    
    set fileinfo [::EventLog::_getSegmentInfo]
    
    $::EventLog::statusBarManager setMessage $::EventLog::statusbar \
        "Recording data for Run: $run : \
[lindex $fileinfo 0] segments found totalling [lindex $fileinfo 1] Mbytes"
    
    set ::EventLog::statusUpdateId \
        [after $repeatInterval [list ::EventLog::_setStatusLine $repeatInterval]]
}
##
# ::EventLog::_duplicateRun
#
#  @return boolean true if the run we are about to write already exists.
#          we're going to define 'exists' as having a run directory in the
#          experiment view.
#
proc ::EventLog::_duplicateRun {} {
    
    set run [::ReadoutGUIPanel::getRun]
    
    # Two possibilities;  If the run was properly ended, there will be a
    # run directory in the experimnent view
    
    set runDirPath [::ExpFileSystem::getRunDir $run]
   
   # If the run was improperly ended, there could be event segments in the
   # current directory.  We'll look for them with glob.
   #
   
   set checkGlob [::ExpFileSystem::getCurrentRunDir]
   set checkGlob [file join $checkGlob [::ExpFileSystem::genEventfileBasename $run]*.evt ]
   
   set eventSegments [llength [glob -nocomplain $checkGlob]]
   
    return [expr {[file exists $runDirPath] || ($eventSegments > 0)}]
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
        if {[::EventLog::_duplicateRun]} {
            error "Run already has event data or segments"
        }
        ::EventLog::_cdCurrent
 
        # Ensure there are no stale synch files:

        file delete -force .exiting
        file delete -force .started
        
        ::EventLog::_startLogger
        ::EventLog::_waitForFile .started $::EventLog::startupTimeout \
                $::EventLog::filePollInterval
        set ::EventLog::expectingExit 0
        ::EventLog::_setStatusLine 2000
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
    
    
    if {$::EventLog::loggerPid ne -1} {
        set ::EventLog::expectingExit 1
        ::EventLog::_waitForFile .exited $::EventLog::shutdownTimeout \
            $::EventLog::filePollInterval
        set ::EventLog::loggerPid -1
        ::EventLog::_finalizeRun
        file delete -force .exited;   # So it's not there next time!!
        
        # Incremnt the run number:
        
        ReadoutGUIPanel::incrRun
        ReadoutGUIPanel::normalColors
        
        if {$::EventLog::statusUpdateId != -1} {
            after cancel $::EventLog::statusUpdateId
            set EventLog::statusUpdateId -1
            $::EventLog::statusBarManager setMessage $::EventLog::statusbar \
                {Run ended}
        }
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
    
    # Create our status bar... just a long label that we'll update
    # every second when runs are active.
    
    set ::EventLog::statusBarManager      [::StatusBar::getInstance]
    set ::EventLog::statusbar \
        [$::EventLog::statusBarManager addMessage {No Event Segments yet}]
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
#      -usensrcs - Boolean... start eventlogger with --number-of-sources switch
#      -additionalsrcs - integer - number of sources in the event builder not
#                    managed by the data source manager.
#      -forcerun  - Boolean... force the run number from GUI rather than using the
#                   one in the begin event (used if no sources provide begin events.)
#

snit::widgetadaptor EventLog::ParameterPrompter {
    component additionalSources
    
    option -logger 
    option -ring
    option -usensrcs          -configuremethod _enableDisableAdditionalSources
    option -additionalsources -configuremethod _setAdditionalSources \
                              -cgetmethod      _getAdditionalSources
    option -forcerun
    
    
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
        
        set options(-logger)            [::DAQParameters::getEventLogger]
        set options(-ring)              [::DAQParameters::getEventLoggerRing]
        set options(-usensrcs)          [::DAQParameters::getUseNsrcsFlag]
        set options(-additionalsources) [DAQParameters::getAdditionalSourceCount]
        set options(-forcerun)          [DAQParameters::getRunNumberOverrideFlag]
        
        

        
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
        
        message $win.help -text "
The next three settings are a bit advanced as they have to do with multiple \
source and event building where some of the sources are not NSCLDAQ sources. \n
'Use  --number-of-sources' should normally be checked if you are using the \
event logger from nscldaq-11.0 or later but not checked if you need to use an \
earlier event logger.  Use 'Additional sources' to adjust the number of end run \
events to expect.  If 0, --number-of-sources is set to the number of event \
sources you specified to this program.  This parameter can be negative if \
some of the sources we're controlling don't produce end of run events. \n
Check the 'Use GUI Run number' if none of your data sources produce a begin \
run event from which the event file name can be derived or if the run numbers they \
do produce are not those the GUI requests.  Note again, this requires the \
NSCLDAQ-11.0 eventlog program or later. "
        
        set f [ttk::frame $win.sourceparams]
        ttk::checkbutton $f.usensrcs -variable [myvar options(-usensrcs)] \
            -onvalue 1 -offvalue 0 -text {Use --number-of-sources} \
            -command [mymethod _updateAdditionalSources]
        ttk::label       $f.adsrclabel -text {Additional Sources}
        install additionalSources using \
            ttk::spinbox $f.additionalsources -from -10 -to 10 -increment 1 \
                -width 4
        $f.additionalsources set $options(-additionalsources)
        $self _updateAdditionalSources
        
    
        ttk::checkbutton $win.forcerun -text {Use GUI Run number} \
            -variable [myvar options(-forcerun)] -onvalue 1 -offvalue 0
        
        grid $win.loglabel $win.logger $win.browselogger -sticky w
        grid $win.datasourcelabel $win.datasource $win.knownrings -sticky w
        grid $win.help -columnspan 3 -sticky ew
        
        grid $f.usensrcs          -row 0 -column 0 -sticky w
        grid $f.adsrclabel        -row 0 -column 1 -sticky w -padx 30
        grid $f.additionalsources -row 0 -column 2 -sticky e 
        grid $f -columnspan 3     -sticky nsew
        
        grid $win.forcerun
        
        $self configurelist $args
        
    }
    #------------------------------------------------------------------------
    # Configuration handlers:
    #
    
    ##
    # _enableDisableAdditionalSources
    #
    #   Enables or disables the aditionalSources compoment depending on
    #   the state of the new value of -usensrcs
    #
    # @param optname - Name of option being configured.
    # @param value   - new value for the option
    #
    method _enableDisableAdditionalSources {optname value} {
        set options($optname) $value
        $self _updateAdditionalSources
    }
    
    ##
    # _setAdditionalSources
    #
    #  Called to configure a new number of sources.  Sets the spinbox value
    #  from the new option.  There's no real point in maintaining the
    #  options array value as the spinbox will just change out from underneath us
    #  so we use a cget handler (See _getAdditionalSources below)
    #
    # @param optname - name of the option being configured.
    # @param value   - New requested value.
    #
    method _setAdditionalSources {optname value} {
        $additionalSources set $value
    }
    ##
    # _getAdditionalSources
    #
    #   Get the value of the additiona sources spinbox.
    #
    # @param optname - option name -- ignored.
    #
    method _getAdditionalSources optname {
        return [$additionalSources get]
    }
    #------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _updateAdditionalSources
    #
    #  Set the state of the additional sources spinbox depending on the
    #  whether or not that option is enabled.
    #
    method _updateAdditionalSources {} {
        if {!$options(-usensrcs)} {
            $additionalSources configure -state disabled
        } else {
            $additionalSources configure -state normal
        }
    }
    
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
        Configuration::Set EventLogger               [$ctl.f cget -logger]
        Configuration::Set EventLoggerRing           [$ctl.f cget -ring]
        Configuration::Set EventLogUseNsrcsFlag      [$ctl.f cget -usensrcs]
        Configuration::Set EventLogAdditionalSources [$ctl.f cget -additionalsources]
        Configuration::Set EventLogUseGUIRunNumber   [$ctl.f cget -forcerun]
        
        # If we're usin gthe nsrcs flag and it would currently be negative warn:
        
        if {[DAQParameters::getUseNsrcsFlag]} {
            set sm [DataSourcemanagerSingleton %AUTO%]
            set mySources [llength [$sm sources]]
            set adtlSources [DAQParameters::getAdditionalSourceCount]
            set totsrc [expr {$mySources + $adtlSources}]
        
            if {$totsrc < 0} {
                tk_messageBox -parent .eventlogsettings -title {Negative source count} \
                    -icon warning -type ok \
                    -message "Your total source count is negative: $mySources managed by us $adtlSources additional sources -> $totsrc total sources"
            }
            $sm destroy
        }
    }
    destroy .eventlogsettings
}
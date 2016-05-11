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

##
# @file evbRdoCallouts.tcl
# @brief Readout Callouts for event builder/ReadoutGUI.
# @author Ron Fox (fox@nscl.msu.edu)

package provide evbcallouts 1.0
package require snit
package require portAllocator
package require ReadoutGUIPanel
package require Experiment 2.0
package require ring
package require StateManager
package require Thread
package require ringsourcemgr
package require EndrunMon


namespace eval ::EVBC {
    variable registered 0;            # nonzero if the event bundle is registered.
    variable initialized 0
    variable pipefd    "";            # Holds the fd to the pipe to the evbpipeline
    variable evbpids   [list];        # Holds list of PIDS that are the event builder.
    
    # Figure out where we are and hence the root of the daq system:
    # We assume we are in one directory below TclLibs in computing this:
    # this results in
    #  ::EVBC::scriptdir - The director this script is in.
    #  ::EVBC::daqtop    - canonicalized top level directory of DAQ installation.
    #
    variable scriptdir [file dirname [info script]]
    variable daqtop    [file normalize [file join $scriptdir .. ..]]
    
    #
    #  Application options passed to initialize:
    #
    variable applicationOptions ""
    
    # GUI things
    
    variable guiFrame             ""

    variable buildEvents          ""
    variable intermediateRing     ""
    variable intermediateRingName ""
    variable destRing             $::tcl_platform(user)

    variable setsEvtlogSource    false
    
    variable glomTsPolicy        earliest
    
    # Communicating with the output ring monitoring thread:
    
    variable monitorTid            "";            # Thread id
    variable monitorMutex          "";            # Mutex handle for condition variable
    variable monitorCondVar        "";            # Condition variable signalled at end of run.
    
    #  Suffix for the application:
    
        variable appNameSuffix       ""

    # Configuration parameters for the event builder:
    
    variable window             ""
    variable XoffThreshold      ""
    variable XonThreshold       ""
}


#-------------------------------------------------------------------------------
##
# @class EVBC::StartOptions
#
#  This snit type is really just used to provide options and defaults for
#  EBC::start.
#
# OPTIONS:
#    * -teering   - if present provides the name of an intermediate ring that
#                   gets the ordered fragments.
#    * -glombuild - If true glom will build events.
#    * -glomdt    - ticks in the coincidence interval used to build events.
#                   required if -glombuild is true.
#    * -glomtspolicy - Timestamp policy for glom.
#    * -destring  - Final destination ring of glom's output.
#                   defaults to the users's name.
snit::type EVBC::StartOptions {
    option -teering
    option -glombuild false
    option -glomdt 1
    option -glomid 0
    option -glomtspolicy -configuremethod checkTsPolicy -default earliest
    option -destring $::tcl_platform(user)
    
    variable policyValues [list earliest latest average]
    ##
    # checkTsPolicy
    #    must be one of earliest, latest, or average
    #
    # @param optname - name of option in which to save this.
    # @param value   - Proposed new value.
    #
    method checkTsPolicy {optname value} {
        if {$value ni $policyValues} {
            error "--glomtspolicy was $value must be one of {[join $policyValues ", "]}"
        }
        set options($optname) $value
    }
    ##
    # getGlomTsPolicies
    #    Get a  list of the glom timestamp policies.
    #
    method getGlomTsPolicies {} {
        return $policyValues
    }
    
}


#------------------------------------------------------------------------------
##
#  @class EVBC::AppOptions
#
#   Options used to define the state of the high level API:
# OPTIONS
#   *  EVBC::startOptions options are installed as a component.
#   * -gui  - enable or disable the GUI.
#   * -restart - Do or don't restart the event buildeer pipeline each onBegin
#   * -setdestringasevtlogsource - Force the value in dest. ring text entry to
#                override the global eventlog settings when written to
#
snit::type EVBC::AppOptions {
    component startOptions
    
    option -gui     true
    option -restart true
    option -setdestringasevtlogsource false

    delegate option * to startOptions
    delegate method * to startOptions
    
    ##
    # constructor
    #   just installs startOptions and processes any command line parameters:
    #
    constructor args {
        set startOptions [EVBC::StartOptions %AUTO%]
        
        $self configurelist $args
    }


    ## Return a new EVBC::AppOptions that has the same state 
    #
    method clone {} {

      # get all of the options and their values and make a dict of them
      set state [dict create]
      foreach opt [$self info options] {
        set value [$self cget $opt]
        dict set state $opt $value 
      }

      # return a new snit object with the same params
      return [[$self info type] %AUTO% {*}$state]

    }
}

##############################################################################
#
#  Low level API:
#
##############################################################################

#-------------------------------------------------------------------------------
##
# @fn EVBC::start option....
#
#   Start the event builder  An EVBC::StartOptions object is used to hold
#   default options.  These are overidden by any options that are passed on
#   the command line.
#
# @param args Tcl list of options and their values.  Must be an even number
#             of elements.
#
# @note The variable EBVC::pipefd is modified to contain the file descriptor
#       open on stdin of the pipeline on success.
# @exception If incomplete or incorrect options are provided.
#
proc EVBC::start args {
    
    # If the pipeline is already active that's an error:
    
    if {$EVBC::pipefd ne ""} {
        error "EVBC::start event builder pipeline is already active"
    }
    
    # Create the start options modified by the args - syntactically validates too.
    
    set options [EVBC::StartOptions %AUTO%]
    if {[llength $args] > 0} {
         $options configure {*}$args
    }
    EVBC::_ValidateOptions $options

    # construct the pipeline:
    # The head of the pipeline is tclsh so that we can
    # push the startOrderer script into the pipe and maintain
    # reads on the pipe.
    #
    set bindir [file join $EVBC::daqtop bin]
    set ::EVBC::appNameSuffix [clock seconds]
    set orderer [file join $bindir startOrderer]
    set pipecommand "tclsh 2>orderer.err";        # TODO - this should be @TCLSH_CMD@
    
    #  If -teering is not null hook teering into the pipeline:
    
    set intermediateRing [$options cget -teering]
    if {$intermediateRing ne ""} {
        set teering "[file join $bindir teering] --ring=$intermediateRing"
        append pipecommand " | " $teering
    }
    #
    #  Figure out the glom command and hook it in.
    #
    
    set glom "[file join $bindir glom] --dt=[$options cget -glomdt] "
    if {![$options cget -glombuild]} {
        append glom " --nobuild "
    }
    append glom " --timestamp-policy=[$options cget -glomtspolicy] "
    append pipecommand " | $glom"
    #
    #  Ground the pipeline in the -destring 
    #
    set stdintoring "[file join $bindir stdintoring] [$options cget -destring]"
    append pipecommand " | $stdintoring |& cat  "; # The cat captures stderr.
    
    #
    #  Create the pipeline:
    #
    
    
    set EVBC::pipefd [open "| $pipecommand" w+]
    set EVBC::evbpids [pid $::EVBC::pipefd];      # Capture the PIDS.

    fconfigure $EVBC::pipefd -buffering line -blocking 0
    fileevent $EVBC::pipefd readable EVBC::_PipeInputReady
    
    
    # Push the startup script into the pipe:
    
<<<<<<< HEAD
    set infd [open $orderer r]
    while {![eof $infd]} {
        set line [gets $infd]
        puts $EVBC::pipefd $line
    }
=======
    puts $EVBC::pipefd "source $orderer"
>>>>>>> master
    ::flush $EVBC::pipefd
    puts $EVBC::pipefd "set ::OutputRing [$options cget -destring]"
    ::flush $EVBC::pipefd
    puts $EVBC::pipefd "start $::EVBC::appNameSuffix"
    ::flush $EVBC::pipefd
    
    # If any parameters have been set push those out now:
    
    foreach param [list window XoffThreshold XonThreshold] {
        set value [set ::EVBC::$param]
        if {$value ne ""} {
            EVBC::configParams $param $value
        }
    }
    
<<<<<<< HEAD
    close $infd
=======
>>>>>>> master
        

    #
    # Next wait for the event orderer service to become available:

    set where [winfo geometry .]
    toplevel .waiting
    wm geometry .waiting $where
    label    .waiting.for -text "Waiting for event builder to start up"
    pack     .waiting.for
    set ports [::portAllocator create %AUTO]
    set appName [::EVBC::getAppName]
    set hunting $appName
    set found 0
    set me $::tcl_platform(user)
    for {set i 0} {$i < 100} {incr i} {
	set allocations [$ports listPorts]
        puts $allocations
	foreach allocation $allocations {
	    set name [lindex $allocation 1]
	    set owner [lindex $allocation 2]
	    if {($name eq $hunting) && ($me eq $owner)} {
		set found 1
	    }
	}
	if {!$found} {
	    update idletasks
	    after 500
	} else {
	    set i 100
	}
    }
    puts "Found"
    $ports destroy
    puts "Portmgr object destroyed"
    destroy .waiting
    puts "Marked waiting for delete"
    if {!$found} {
	error "Event builder failed to start within timeout"
    }

}

proc EVBC::getAppName {} {
    set me      $::tcl_platform(user)
    return "ORDERER:$me:$::EVBC::appNameSuffix"
}

#------------------------------------------------------------------------------
##
# @fn EVBC::stop
#
#   Stop the event builder pipeline.
#
#  @exception The event builder pipeline is not running.
#
proc EVBC::stop {} {

    EVBC::_CheckPipeline EVBC::stop
    
    # Tell the interpreter to exit.
    # note that the pipefd close will trigger _PipeInputReady which will
    # in turn close the pipefd after reaping any errors.

    set EVBC::evbpids [list];              # Expecting the exit so empty the pidlist.
    
    # Push an exit and mark us not connected.
    
    puts $EVBC::pipefd exit
    ::flush $EVBC::pipefd
                   
    
}
#------------------------------------------------------------------------------
##
# @fn EVBC::reset
#
#  Reset the timestamps etc. in the event builder.
#  This initializes the concept of the eventbuilders history so it loses all
#  memory of ever seeing a fragment.
#
# @exception The event buidler pipeline is not running.
#
proc EVBC::reset {}  {
    EVBC::_CheckPipeline EVBC::reset
    puts $EVBC::pipefd EVB::reset
}
#------------------------------------------------------------------------------
##
# @fn EVBC::flush
#
#  Forces all event builder queues to flush through to the output.
#
#  @exception The event builder pipeline is not running.
#
proc EVBC::flush {} {
    #EVBC::_CheckPipeline EVBC::flush
    
    #puts $EVBC::pipefd EVB::flushqueues
}

##
# getOrdererPort
#   Returns the event orderer port number
#
proc EVBC::getOrdererPort {} {
   #
    #  Figure out what port the event builder is running on... or if it's running
    #
    set portManager [::portAllocator create %AUTO%]
    set allocations [$portManager listPorts]
    set user $::tcl_platform(user)
    set appName "ORDERER:$user:$::EVBC::appNameSuffix"
    set port ""
    foreach allocation $allocations {
        set name  [lindex $allocation 1]
        set owner [lindex $allocation 2]
        if {($name eq $appName) && ($owner eq $user)} {
            set port [lindex $allocation 0]
            break
        }
    }
    $portManager destroy
    
    if {$port eq ""} {
        error "EVBC::startRingSource Unable to locate the event builder service"
    }
    return $port    
}


#------------------------------------------------------------------------------
## @fn EVBC::registerRingSource
#
# Adds a ring source to the RingSourceMgr callout bundle. This is actually just
# a convenience method so that people can remain dealing with only the EVBC::
# namespace. In the end, the EVBC callout bundle does almost nothing for setting up 
# the ring sources.
#
# @param sourceRingUrl          - URL of the source ring.
# @param timestampExtractorLib  - Path to the timestamp extractor shared object.
# @param id                     - Source id used to associate event fragments
#                                 with an input queue.
# @param info                   - Long description used to identify the source
#                                 in the event orderer GUI.
<<<<<<< HEAD
=======
# @param expectHdrs             - All data must have body headers. Timestamp lib
#                                 and id can be left as empty strings
# @param oneshot                - If provided number of ends that result in exit.
# @param timeout                - If provided, timeout in seconds after first end to 
#                                 wait for all ends in --oneshot mode.
# @param offset                 - Optinoal time offset.
>>>>>>> master
# @note Event sources are subprocesses of us but not subprocesses of the
#       the event building pipeline.
#
#
<<<<<<< HEAD
proc ::EVBC::registerRingSource {source lib id info {expectHdrs 0}} {
   ::RingSourceMgr::addSource $source $lib $id $info $expectHdrs
=======
proc ::EVBC::registerRingSource {source lib id info {expectHdrs 0} {oneshot {}} {timeout {}} {timeoffset 0}} {
   ::RingSourceMgr::addSource $source $lib $id $info $expectHdrs $oneshot $timeout $timeoffset
>>>>>>> master
}

#------------------------------------------------------------------------------
## @fn EVBC::registerS800Source
#
# Convenience method for registering an S800 source. It is just a call to 
# EVBC:registerRingSource but the s800 timestamp extractor is provided for
# the user.
#
# @param ringUrl          - URL of the source ring.
# @param id               - Source id used to associate event fragments
#                           with an input queue.
# @param desc             - Long description used to identify the source
#                           in the event orderer GUI.
#
proc ::EVBC::registerS800Source {ringUrl id {desc {S800 USB data}}} { 
    #
    # Figure out the timestamp extractor path:
    #
    set extractor  [file join $EVBC::daqtop lib libS800TimeExtractor.so]
    
    ::EVBC::registerRingSource $ringUrl $extractor $id $desc
}
#------------------------------------------------------------------------------
##
# @fn EVBC::startRingSource [obsolete - use RingSourceMgr::startSource for new apps]
#
#   Starts a ring buffer event source
#
# @param args -set of parameters expected by ::RingSourceMgr::startSource
# @note Event sourcese are subprocesses of us but not subprocesses of the
#       the event building pipeline.
#
proc EVBC::startRingSource {args} {
  ::RingSourceMgr::startSource {*}$args
}


##
# @fn EVBC::startS800Source
#
#  Starts the s800 fragment source.
#
# @param ringUrl - URL of ring gettting S800 data.
# @param id      - Id of data source.
# @param desc    - short description defaults to "S800 USB data".
#
proc EVBC::startS800Source {ringUrl id {desc {S800 USB data}}} {
    #
    # Figure out the timestamp extractor path:
    #
    set extractor  [file join $EVBC::daqtop lib libS800TimeExtractor.so]
    
    EVBC::startRingSource $ringUrl $extractor $id $desc
    
}

###############################################################################
#
# High level API
#
##############################################################################

#-----------------------------------------------------------------------------
##
# EVBC::initialize 
#
#  For the most part this just creates, populates and validates the
#  initialization options.  The options are maintained in an EVBC::AppOptions
#  object stored in EVBC::applicationOptions
#
# @param args - Optional configuration options.
#
proc EVBC::initialize args {
    #
    # Create and optionally configure the application objects.
    #
    if {!$EVBC::initialized} {
        set EVBC::initialized true
        set EVBC::applicationOptions [EVBC::AppOptions %AUTO%]
        
        if {[llength $args] > 0} {
            $EVBC::applicationOptions configure {*}$args
        }
        EVBC::_ValidateOptions $EVBC::applicationOptions
        
        # if -gui is true, start it and paste it:
        
        if {[$EVBC::applicationOptions cget -gui] && [$EVBC::applicationOptions cget -restart]} {
            EVBC::_StartGui
        }

        # if -gui is true, start it
        if {[$EVBC::applicationOptions cget -gui] && ($EVBC::guiFrame eq "")} {
            EVBC::_StartGui
        }

         
    }
    #
    #  If the app is being destroyed kill the event builder too:
    
    bind . <Destroy> +[list EVBC::_Exiting %W]
    
}
#------------------------------------------------------------------------------
##
# @fn EVBC::onBegin
#
#   Call this from the user's OnBegin method.
#   -  If -restart is true kills the event builder and vwaits for the pipefd
#      to change.
#   -  If the fd is empty indicating the event builder is not running, starts it.
#   - If the user defined a startEVBSources proc and we started the event builder
#     invoke it.
#   - If the UI exists, then disable it completely.
#
proc EVBC::onBegin {} {
    if {$EVBC::applicationOptions eq ""} {
        error "OnStart has not initialized the event builder package."
    }
    if {[$EVBC::applicationOptions cget -restart] && ($EVBC::pipefd ne "")} {
        EVBC::stop
        # reset the sources so that we don't skip them.
        ::RingSourceMgr::resetSources
        vwait EVBC::pipefd;      # Will become empty on exit.
    }
    catch [list ringbuffer create $EVBC::destRing] msg;  #ensure ring exists first.
    
    
    # IF needed, create the destination and the intermediate ring:

    set teering [$EVBC::applicationOptions cget -teering]
    if {$teering ne ""} {
	catch {ringbuffer create $teering}
    }
    set destring [$EVBC::applicationOptions cget -destring]
    if {$destring ne ""} {
        catch {ringbuffer create $destring}
    }
    
    #  If needed restart the EVB and disable the GUI...if it exists
    
    if {$EVBC::pipefd eq ""} {
        EVBC::start \
            -teering   $teering   \
            -glombuild [$EVBC::applicationOptions cget -glombuild] \
            -glomdt    [$EVBC::applicationOptions cget -glomdt]    \
            -glomtspolicy [$EVBC::applicationOptions cget -glomtspolicy] \
            -destring  $destring
        
        
        if {$EVBC::guiFrame ne ""} {
            EVBC::_DisableGUI 
        }
            
        if {[info commands startEVBSources] ne ""} {
          ::EVBC::_waitForEventBuilder
          startEVBSources
<<<<<<< HEAD
=======
          
            # Before allowing the reaout(s) to start we want to wait a bit to be
            # sure the sources are connected to the ring master and all that rot.
            # this is needed in case the begin run action is quick.  We'll wait
            # two seconds for now.  In testing with systems that essentially don't
            # have initialization, I observed that sometimes the begin run events
            # would get missed and this is why I think it happens.
        
            after 2000
          
>>>>>>> master
        }
    } else {
	EVBC::reset
    }
    
}
#------------------------------------------------------------------------------
##
# @fn EVBC::onEnd
#
#  Call this from the user's OnEnd method.
#   - Flushes the event builder.
#   - reenables the gui if it exists.
proc EVBC::onEnd {} {
    catch {EVBC::flush} ;  # Catch in case the evb exited badly.
    
    # Wait for the monitor thread to signal the end runs balanced the begin runs:
    
    
    
    if {$EVBC::guiFrame ne ""} {
            EVBC::_EnableGUI
    }
}

#-------------------------------------------------------------------------------
# @fn EVBC::configParams
#    Send dynamic configuration options (EVB::config command).
#    If the event builder is actually running, the config param is sent now
#    If not, the config param is sent on all subsequent event builder starts.
#
# @param parameter - the parameter to configure must be one of
#                    * window set number of seconds in the build window.
#                    * XoffThreshold - set the number of queued bytes before xoffing.
#                    * XonThreshold  - set then umber of queued bytes at which XON
# @param value    - A new positive integer value (all config parameters above take
#                   positive integers).
#
# @note - it is an error (not caught) for XonThreshold to be larger than Xoffthreshold.
# @note - I actually anticipate the user is going to mostly adjust the build window
#         so that the event builder latency gets better matched to its sources.
#
proc EVBC::configParams {parameter value} {
    
    # Validate the parameter name:
    
    set configParams [list window XoffThreshold XonThreshold]
    if {$parameter ni $configParams} {
        error "EVBC::configure $parameter must be one of [join $configParams {, }]"
    }
    # Validate the parameter value:
    
    if {![string is integer -strict $value]} {
        error "EVBC::configure $parameter value $value must be an integer and is not"
    }
    if {$value <= 0} {
        error "EVBC::configure $parameter value $value must be strictly > 0"
    }
    
    set EVBC::$parameter $value;         # Send next start.
    
    ## Send now if we can.
    
    if {$::EVBC::pipefd ne ""}  {
        # send the command and flush the pipe:
        puts "Sending: EVB::config set $parameter $value"
        puts $EVBC::pipefd "EVB::config set $parameter $value"
        ::flush $EVBC::pipefd
    }
}
    

#------------------------------------------------------------------------------
## 
# @fn EVBC::isRunning
#
# Checks whether there is a channel associated with the event builder. Notice that
# if the eof occurs, the EVBC::pipefd gets reset to "".
#
# @returns boolean
# @retval 0 - EVB is not running
# @retval 1 - EVB is running
proc EVBC::isRunning {} {
  return [expr {$::EVBC::pipefd ne ""}]
}
###############################################################################
#
# Private procs.
#
###############################################################################


#------------------------------------------------------------------------------
##
# @fn [private] EVBC::_CheckPipeline
#
#   Errors if the event builder pipeline is not running
#
# @param msgPrefix - prefixes the error message.
#
proc EVBC::_CheckPipeline {msgPrefix} {
    if {$EVBC::pipefd eq ""} {
        error "$msgPrefix the event buider pipleline is not running"
    }
}
#------------------------------------------------------------------------------
##
# @fn [private] EVBC::_PipeInputReady
#
# Called when the event builder pipeline has input ready.
# Read the input line and _Output it.
# TODO:  This should go to the output screen of the ReadoutGUI.
#s
# If the EOF is reached close the pipe and collect the error message as well.
#
proc EVBC::_PipeInputReady {} {
    if {[eof $EVBC::pipefd]} {
        catch {close $EVBC::pipefd} msg
        EVBC::_Output "Event builder pipeline exited $msg"
        set EVBC::pipefd ""
	#
	# Ensure the entire pipeline is dead too, and bitch if this is unexpected:
	#
	if {[llength $::EVBC::evbpids] != 0} {
	    tk_messageBox -icon error -title {EVB pipe exit}  -type ok \
		-message {An element of the event builder pipeline exited.  Killing the entire pipe}
	    foreach pid $::EVBC::evbpids {
		catch {exec kill -9 $pid}
	    }
	}
    } else {
        EVBC::_Output [gets $EVBC::pipefd]
    }
}

#-------------------------------------------------------------------------------
##
# @fn [private] EVBC::_Output
#
#  Outputs a message.
#  TODO: While debugging this goes to stdout... later it needs to go to the
#  output window of the readoutGUI.
#
# @param msg - the message to output.
#
proc EVBC::_Output msg {
    ReadoutGUIPanel::outputText "[clock format [clock seconds]] $msg"
}

#-----------------------------------------------------------------------------
##
# @fn [private] EVBC::_ValidateOptions
#
#  Does semantic validation of program options and EVBC::start options.
#
# @param options - An option object.  This can be eitherEV
#                  an instance of either EVBC::StartOptions or EVBC::AppOptions
#
proc EVBC::_ValidateOptions options  {
    
    # If not -glombuild put in a default -glomdt of 1 since it's ignored:
    
    if {![$options cget -glombuild]} {
        $options configure -glomdt 1
    }
    #  -glomdt is required
    
    if {[$options cget -glomdt] eq ""} {
        error "EVBC::start -glomdt must be given a value"
    }
    
}
#-------------------------------------------------------------------------
##
# @fn EVBC::_StartGui
#
#  Create the event builder GUI and glue it into the ReadoutGUI frame.
#  TODO: For now this all goes into a toplevel .evgui
#
#   The GUI contains:
#   *  A checkbox that enables/disables glom building.
#   *  A glom --dt spinner.
#   *  An entry for the destination ring name.
#   *  A checkbox to enable an intermediate fragmen ring.
#   *  An entry for the name of that ring.
#
proc EVBC::_StartGui {} {
    
    # TODO: window should be a frame in readout GUI and
    #       gridded into the bottom  most row.
    
    
    set window [frame .evbgui]
    
    # The Glom control frame:
    
    # Glom building or not and coincidence window.
    
    
    set glom [ttk::labelframe  $window.glom -text {Event Building parameters (vsn 11)} \
                      -padding 6]
    
    ttk::checkbutton $glom.build -text {Build} -onvalue true -offvalue false \
        -command [list EVBC::_GlomEnableDisable $glom.build $glom.dt] \
        -variable EVBC::buildEvents
    
    spinbox $glom.dt -from 0 -to 1000000 -increment 1 \
        -command [list EVBC::_GlomDtChanged $glom.dt] -width 8

    # We need to bind the enter key to invoke _GlomChanged as well
    # since no keystrokes fire the -command.

    bind $glom.dt <Key-Return> [list EVBC::_GlomDtChanged %W]
    bind $glom.dt <Key-Tab>    [list EVBC::_GlomDtChanged %W]
 
    ttk::label $glom.dtlabel -text "Coincidence interval"

    
    grid $glom.build $glom.dt $glom.dtlabel
    
    # Glom timestamp policy:
    
    set policies [$::EVBC::applicationOptions getGlomTsPolicies]
    ttk::label $glom.plabel -text "Ts is: "
    grid $glom.plabel -row 1 -column 0 -padx 5 -pady 3
    set col 1
    foreach policy $policies {
        radiobutton $glom.$policy \
            -variable ::EVBC::glomTsPolicy -value $policy -text $policy \
            -command [list $EVBC::applicationOptions configure -glomtspolicy $policy]
        grid $glom.$policy -row 1 -column $col -pady 3
        
        incr col
    }
    
    grid $glom -row 0 -column 0 -sticky nsew
    
    #  Enable/disable intermediate ring and which ring it is:
    
    set intermediate [ttk::labelframe $window.intermediate -text {Ordered Fragment Ring} \
                          -padding 6]
    ttk::checkbutton $intermediate.enable -text {Tee output to this ring}\
        -onvalue true -offvalue false \
        -command [list EVBC::_IntermediateEnableDisable $intermediate.enable $intermediate.ringname] \
        -variable EVBC::intermediateRing
    ttk::entry  $intermediate.ringname -width 15 \
        -textvariable EVBC::intermediateRingName
    ttk::label  $intermediate.ringlbl -text "Ring Name"
    

    trace add variable EVBC::intermediateRingName write EVBC::_IntermediateRingChanged
    set EVBC::intermediateRing false
    
    EVBC::_IntermediateEnableDisable $intermediate.enable $intermediate.ringname
    
    grid $intermediate.enable -columnspan 2 -sticky w -padx 5 -pady 3
    grid $intermediate.ringlbl $intermediate.ringname -padx 5 -pady 3
    grid $intermediate -row 0 -column 1 -sticky nsew
    
    
    set destringWin [ttk::labelframe $window.dest -text {Destination Ring} \
                        -padding 6]
    ttk::label $destringWin.ringlbl -text {Name}
    ttk::entry $destringWin.ringname -textvariable EVBC::destRing -width 15
    trace add variable  EVBC::destRing  write \
        [list EVBC::_ChangeDestRing]
    ttk::checkbutton $destringWin.change -variable EVBC::setsEvtlogSource \
                     -onvalue true -offvalue false -text {Use for recording}
    trace add variable EVBC::setsEvtlogSource write EVBC::_ChangeSetsEvtlogSource
    
    grid $destringWin.ringlbl -row 0 -column 0 -sticky e -padx 5 -pady 3 
    grid $destringWin.ringname -row 0 -column 1 -sticky w -padx 5 -pady 3
    grid $destringWin.change -columnspan 2  -padx 5 -pady 3
    grid $destringWin -row 0 -column 2 -sticky nsew
    

    # Figure out where to grid the GUI:
    
    set dimensions [grid size .]
    set rows       [lindex $dimensions 1]
    set cols       [lindex $dimensions 0]
    incr rows
    grid $window  -row $rows -column 0 
    
    set EVBC::guiFrame $window
    
    ::EVBC::updateGuiFromOptions
    
}
##
# updateGuiFromOptions
#
#   Sets the value of the event builder GUI from its current option values
#   It is up to the caller to ensure that the GUI and its fields exist.
#
proc ::EVBC::updateGuiFromOptions {} {
    
    if {[$EVBC::applicationOptions cget -glombuild]} {
        set ::EVBC::buildEvents true
    } else {
        set ::EVBC::buildEvents false
    }


    $::EVBC::guiFrame.glom.dt config -state normal
    $::EVBC::guiFrame.glom.dt delete 0 end
    $::EVBC::guiFrame.glom.dt  insert 0 [$EVBC::applicationOptions cget -glomdt]
    
    
    ::EVBC::_GlomEnableDisable \
        $::EVBC::guiFrame.glom.build \
        $::EVBC::guiFrame.glom.dt;     # Set initial enable/disable of spinbox.    

    
    set teering [$EVBC::applicationOptions cget -teering]
    set EVBC::intermediateRingName $teering

    if {$teering eq ""} {
        set EVBC::intermediateRing false
    } else {
        set EVBC::intermediateRing true
    }
    
    set EVBC::destRing [$EVBC::applicationOptions cget -destring]

}
#-----------------------------------------------------------------------------
##
# @fn EVBC::_GlomEnableDisable
#
#   Called when the glom enable/disable checkbox changed.  If the checkbox is
#   unchecked we need to disable the spinbox and vica versa.
#
#
# @param checkbox  - Checkbox widget path.
# @param dt        - Path to dt spinner.
#
proc EVBC::_GlomEnableDisable {checkbox dt} {
    if {$EVBC::buildEvents} {
        $dt configure -state normal
    } else {
        $dt configure -state disabled
    }
    $EVBC::applicationOptions configure -glombuild $EVBC::buildEvents
}
#-------------------------------------------------------------------------------
##
# @fn EVBC::_GlomDtChanged
#
#  Called when the build time for glom changed.  Get the new value
#  and update the configuration.
#
# @param dt - Widget that has the spinbox.
#
proc EVBC::_GlomDtChanged {dt} {
    $EVBC::applicationOptions configure -glomdt [$dt get]
}
#------------------------------------------------------------------------------
##
# @fn EVBC::_IntermediateEnableDisable
#
#  Called when the intermediate ring enable checkbox changes state.
#  This is bound to EVBC::intermediateRing so the state is reflected there.
#
# @param enable - Widget of the enable checkbutton.
# @param ring   - Widget that has the ring name.
#
proc EVBC::_IntermediateEnableDisable {enable ring} {
    if {$EVBC::intermediateRing} {
        $ring config -state normal
        $EVBC::applicationOptions configure -teering $EVBC::intermediateRingName
    } else {
        $ring config -state disabled
        $EVBC::applicationOptions configure -teering ""
    }
}
#------------------------------------------------------------------------------
##
# @fn EVBC::_IntermediateRingChanged
#
#  Trace called when the intermediate ring name has changed.
#  EVBC::intermedateRingName has the new value
#
#  This is a variable trace handler.
#
proc EVBC::_IntermediateRingChanged {name index op} {
    $EVBC::applicationOptions configure -teering $EVBC::intermediateRingName   
}
#------------------------------------------------------------------------------
##
# @fn EVBC::_EnableGUI
#
#  Enbable the gui.
#  To start with all non terminal states are set normal
#  Once that's done we need to selectively disable elements depending on the state
#  of the UI itself.
#
#
proc EVBC::_EnableGUI {} {
    EVBC::_SetGuiState $EVBC::guiFrame normal
    
    EVBC::_GlomEnableDisable $EVBC::guiFrame.glom.build $EVBC::guiFrame.glom.dt
    EVBC::_IntermediateEnableDisable \
        $EVBC::guiFrame.intermediate.enable \
        $EVBC::guiFrame.intermediate.ringname
}
#-----------------------------------------------------------------------------
##
# @fn EVBC::_DisableGui
#
#  Disables the GUI that is rooted in $EVBC::guiFrame
#  This is pretty easy.. we just need to set the state of all terminal
#  widgets to disabled:
#
proc EVBC::_DisableGUI {} {
    EVBC::_SetGuiState $EVBC::guiFrame disabled
}
#-----------------------------------------------------------------------------
##
# @fn EVBC::_SetGuiState state
#
#  Set the state of all terminal widgets of the GUI widget tree to the requested
#  state.
#
# @param widget  top level of the hierarchy to affect.
# @param desired state
#
proc EVBC::_SetGuiState {widget state} {
    foreach w [winfo children $widget] {
        if {[llength [winfo children $w]] > 0} {
            EVBC::_SetGuiState $w $state
        } else {
            $w configure -state $state
        }
    }
}
#----------------------------------------------------------------------------
##
# @fn EVBC::_ChangeDestRing
#
#  Called when the name of the destination ring changes.
#
#  This is a variable trace handler.
#
proc EVBC::_ChangeDestRing {name index op} {
    $EVBC::applicationOptions configure -destring $EVBC::destRing
    if {[$EVBC::applicationOptions cget -setdestringasevtlogsource] } {
      ::Configuration::Set EventLoggerRing "tcp://localhost/$EVBC::destRing"
    }
}

proc EVBC::_ChangeSetsEvtlogSource {name index op} {
    set oldring [::Configuration::get EventLoggerRing]
    $EVBC::applicationOptions configure -setdestringasevtlogsource $EVBC::setsEvtlogSource 
    if { [ $EVBC::applicationOptions cget -setdestringasevtlogsource ] } {

        ::Configuration::Set EventLoggerRing "tcp://localhost/$EVBC::destRing"
    }
    set newring [::Configuration::get EventLoggerRing]
    ReadoutGUIPanel::Log EVB log "Eventlog source changed $oldring --> $newring"
}

#------------------------------------------------------------------------------
##
# @fn EVBC::_Exiting
#
#   Called when we're exiting.. if the event builder is running
#   kill it off:
# @param w - the widget that's being destroyed.
#             We wish we could care about . but that's caught by the main stuff.
#
proc EVBC::_Exiting w {
    if {($w eq ".runnumber") && ($EVBC::pipefd ne "")} {
        EVBC::stop

    }
}
#------------------------------------------------------------------------------
##
# @fn EVBC::_waitForEventBuilder
#
#  Waits for the event builder to be ready to accept TCP/IP connections
#  By now it's assumed the event builder is visible to the port manager.
#
#
proc EVBC::_waitForEventBuilder {} {

    # Figure out which port the event builder is listening on:

    set ports [::portAllocator create %AUTO%]
    set me $::tcl_platform(user)
    set name [EVBC::getAppName]
    set port -1
    set allocations [$ports listPorts]
    foreach allocation $allocations {
	if {([lindex $allocation 1] eq $name) && ([lindex $allocation 2] eq $me)} {
	    set port [lindex $allocation 0]
	    break
	}
    }
    $ports destroy

    #  Try to connect every 100ms until success:

    while {[catch {socket localhost $port} fd]} {
	after 100
    }
    close $fd
}


#-------------------------------------------------------------------------------
#
#  The code below provides a state machine bundle that is now the preferred
#  way to register this code.  Note that the user is still going to have
#  to provide callbaks to start the event sources.
#  TODO:  Maybe each data source provider can include a call that will
#         know how to start an associated evb data source for each instance
#         it manages?
#


##
#  EVBC::attach
#    Called when the bundle is attached to the state manager
#    Initialize the event builder. 
#
# @param state - the current state.
#
# @note you can use EVBC::configure to set the configuration of the
#       manager.  This includes enabling the GUI and so on.
#
proc ::EVBC::attach state {
}
##
# EVBC::enter
#   Called when a new state is entered.
#   * Active -> Halted  invoke onEnd
# @param from - state that we left.
# @param to   - State that we are entring.
#
proc ::EVBC::enter {from to} {
    if {($from eq "Active") && ($to eq "Halted")} {
        ::EVBC::onEnd
    }
<<<<<<< HEAD
    if {($from in [list Active Paused]) && ($to eq "NotReady")} {
        ::EVBC::stop
    }

=======
    if {$to eq "NotReady"} {
        catch {::EVBC::stop}
    }
>>>>>>> master
}
##
# EVBC::leave
#   Called when a state is being left.
#   * Halted -> Active   invoke onBegin - we do this inleave to get the jump
#     on when the data sources start spewing data.
#
# @param from - State we are leaving.
# @param to   - State we are about to enter.
#
proc ::EVBC::leave {from to} {
    if {($from eq "Halted") && ($to eq "Active")} {
        ::EVBC::onBegin
    }
}


##
#  useEventBuilder
#     Register the event builder with the state manager
#
#     This also adds the RingSourceMgr immediately afterwards to work with
#     it.
#
#
proc EVBC::useEventBuilder {} {

    if {$EVBC::registered == 0} {
        set stateMachine [RunstateMachineSingleton %AUTO%]
        set callouts [$stateMachine listCalloutBundles]
        $stateMachine addCalloutBundle EVBC [lindex $callouts 0]
        set ::EVBC::registered 1
        $stateMachine addCalloutBundle ::RingSourceMgr [lindex $callouts 0]
        ::EndrunMon::register [lindex $callouts 0]
    }
    
}

##
#  configure
#     Configure the event builder options.
#
# @param args - configuration parameters.  This is a list
#               of alternating option and option values.
#
# @note The event builder must have already been registered via EVBC::useEventBuilder
# @note If necessary the GUI is added/removed.
#
proc ::EVBC::configure args {
    
    # First configure
    
    $::EVBC::applicationOptions  configure {*}$args
    
    # Now see what we need to do with the GUI (start/stop leave alone).
    
    set gui [$::EVBC::applicationOptions cget -gui]
    if {$gui && ($::EVBC::guiFrame eq "")} {
        ::EVBC::_StartGui
    }
    
    if {(!$gui) && ($::EVBC::guiFrame ne "")} {
        destroy $::EVBC::guiFrame
        set ::EVBC::guiFrame ""
    }
    # If the GUI is active we must update its elements
    
    if {$::EVBC::guiFrame ne ""} {
        ::EVBC::updateGuiFromOptions
    }
}

namespace eval ::EVBC {
    namespace export attach enter leave
}


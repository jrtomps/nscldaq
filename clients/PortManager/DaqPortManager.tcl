#
#   This is the application itself.  We are just going to setup the
#   pieces and parts of the system, hook them together and enter the event loop.
#
#   Since we have no idea which version of tcl will be used to run us, we require that
#   the user run us as tclshx.y Application.tcl ...
#

#   Usage:
#      tclshx.y TclServer.tcl [-ports range] [-listen port] [-log file]
#
#  If being run from root, in general the defaults are correct for all of these:
#      -ports  30001-31000
#      -listen 30000
#      -log    /var/log/var/log/nscldaq/portmanager.log
#
 
# In order to find the packages we need to add our directory to auto_path:

set here [file dirname [info script]]
lappend auto_path $here

package require snit
package require ConnectionManager
package require PortManager
package require Log

#  Global variables that hold the defaults:

set defaultListenPort   30000
set defaultPortRange   {30001 31000}
set defaultLogFile      /var/log/nscldaq/portmanager.log
set defaultUsageFile    /var/tmp/daqportmgr/ports.txt
set defaultPortFile     /var/tmp/daqportmgr/listen.port
set defaultPidFile      /var/run/nscldaq/daqportmgr.pid

#  These global variables hold the selected run parameter values:

set selectedListenPort  $defaultListenPort
set selectedPortRange   $defaultPortRange
set selectedLogFile     $defaultLogFile
set selectedUsageFile   $defaultUsageFile
set selectedPortFile    $defaultPortFile
set selectedPidFile     $defaultPidFile

# usage:
#    Produces a return value that contains the program
#    usage string.
#
proc usage {} {
    set Usage         {PortManager.tcl ?-ports low-high? ?-listen port? ?-log filename?}
    append Usage "\n" {    -ports   - Specifies the range of ports to be managed}
    append Usage "\n" {    -listen  - Specifies the port on which the program listens for connections}
    append Usage "\n" {    -log     - Specifies the log file}
    
    return $Usage
}

# ParseParameters:
#   Parse the command line parameters.  Since we don't support  anything
#   other than switches with parameters this is relatively easy.
# Parameters:
#   rawParameters   (list)
#       The list of raw command line parameters.
# SideEffects:
#   The global variables selected* get set according to the mix of default
#   values and switched values.
#
proc ParseParameters {rawParameters} {
    global  selectedListenPort
    global  selectedPortRange
    global  selectedLogFile
    
    # Parameters come in pairs:
    
    for {set i 0} {$i < [llength $rawParameters]} {incr i 2} {
        set Switch [lindex $rawParameters $i]
        set Value  [lindex $rawParameters [expr {$i+1}]]
        switch -exact $Switch {
            -ports {
                #  Ports must have a list with 2 numeric elements...
                
                if {[scan $Value "%d-%d" low high] != 2} {
                    error "Failed to parse -ports $Value  \n [usage]"
                }
                #  If necessary swap values to get low hi pairs:
                
                if {$low < $high} {
                    set tmp $high
                    set high $low
                    set low  $tmp
                }
                set selectedPortRange [list $low $high]
                
            }
            -listen {
                # must have a single numerical port value:
                
                if {[scan $Value "%d" port] != 1} {
                    error "Failed to parse -listen $Value \n [usage]"
                }
                set selectedListenPort $port
            }
            -log {
                # The value is a text string:
                
                set selectedLogFile $Value
            }
            default {
                error "Invalid switch $Switch\n [usage]"
            }
        }
       
    }
    
}
# enterEventLoop
#   Enters the event loop:  This  program is built to run
#   from tclsh, rather than wish, but is event driven.
#   The simple code below makes tclsh process events
#   until the application is forced to exit:
#
set forever 0
proc enterEventLoop {} {
    global forever
    vwait  forever
}
# run:
#   Called to run the application.  Any initialization is done first,
#   then this proc is called; to instantiate instances of the components
#   and to stitch them together.
# Parameters:
#   argList (list)
#       Program parameter list e.g. from command line.
#
proc run {argList} {
    global selectedListenPort
    global selectedPortRange
    global selectedLogFile
    global selectedUsageFile
    global selectedPortFile
    global selectedPidFile
    
    ParseParameters $argList
    
    # Create the directories needed by all the files:
    
    file mkdir [file dirname $selectedLogFile]
    file mkdir [file dirname $selectedUsageFile]
    file mkdir [file dirname $selectedPortFile]
    file mkdir [file dirname $selectedPidFile]
    
    # Set our port and pid files:
    
    set   fd [open $selectedPortFile w]
    puts $fd $selectedListenPort
    close $fd
    
    set   fd [open $selectedPidFile w]
    puts $fd [pid]
    close $fd
    
    #  Build the Log and port manager objects:
    
    puts "log: $selectedLogFile"
    
    set logger [Log create log -filename $selectedLogFile]
    set ports  [PortManager create ports -range $selectedPortRange]
    set cm     [ConnectionManager create cm -ports     $ports \
                                            -logger    $logger \
                                            -listener  $selectedListenPort \
                                            -usagefile $selectedUsageFile]
                                            
    # Start processing events:
    
    enterEventLoop
}

#  Entry point

run $argv
# 
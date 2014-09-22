
package provide OfflineEVBOutputPipeline 11.0

package require snit
package require InstallRoot 
package require ring
package require DAQParameters 
package require eventLogBundle

snit::type OfflineEVBOutputPipeParams {
  option -ringname     -default "tcp://localhost/OfflineEVBOut"
  option -prefix       -default "reordered"
  option -stagearea    -default [file join $::env(HOME) offlinestagearea]
  option -nsources     -default "2"
  option -logger       -default ""
  option -usensrcs     -default 1
  option -forcerun     -default 0
  option -usechecksum  -default 1 

  constructor {args} {
    set options(-logger) [DAQParameters::getEventLogger]
    $self configurelist $args   

  }


  method validate {} {
    set errors [list]
    $self validateRing errors
    $self validatePrefix errors
    $self validateStagearea errors
    $self validateNSources errors
    $self validateLogger errors

    return $errors 
  }

  method validateRing {errors_} {
    upvar $errors_ errors
    set pattern {^(\w+://)([/]*[\w\.]+)(/[\w\.]+)*$}
    if {[regexp $pattern $options(-ringname) match proto host ring]} {
      puts "match    = $match"
      puts "protocol = $proto"
      puts "host     = $host"
      puts "ring     = $ring"

      if {![info exists ring]} {
        lappend errors "Ring name not specified as a valid proto://host/ring"
      }

      set ring [string trimleft $ring "/"]
      if {![file exists [file join /dev shm $ring]]} {
        lappend errors "Ring \"$ring\" does not exist on localhost." 
      }
    } else {
      lappend errors "Ring name \"$options(-ringname)\" is not formed as proto://host/ring"
    }
  }

  method validatePrefix {errors_} {
    upvar $errors_ errors
      if {$options(-prefix) eq ""} {
      lappend errors "Run file prefix must be a string of nonzero length."
    }
  }

  method validateStagearea {errors_} {
    upvar $errors_ errors
    set dir $options(-stagearea)
    if {![file exists $dir]} {
      lappend errors "Stagearea must be a directory that exists."
    } else {
      set subdir [file join $dir experiment]
      if { (![file exists $subdir]) || (![file owned $subdir]) } {
        lappend errors "Stagearea must contain writable experiment directory"
      } 
      set subdir [file join $dir experiment current]
      if { (![file exists $subdir]) || (![file writable $subdir]) } {
        lappend errors "Stagearea must contain writable experiment/current directory"
      } 
      set subdir [file join $dir complete]
      if { (![file exists $subdir]) || (![file owned $subdir]) } {
        lappend errors "Stagearea must contain writable complete directory"
      }
    }
  }

  method validateNSources {errors_} {
    upvar $errors_ errors
    if {$options(-nsources) < 0} {
      lappend errors "Number of sources for eventlog must be greater than or equal zero."
    }
  }

  method validateLogger {errors_} {
    upvar $errors_ errors
    if {![file exists $options(-logger)]} {
      lappend errors "No eventlog program exists at \"$options(-logger)\""
    }
  }


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

# --------------------------------------------------------------
# Redefine some of the ReadoutGUIPanel information

namespace eval ReadoutGUIPanel {
  variable run

  proc recordData {} { 
    return 1
  }

  proc setRun {number} {
    variable run
    set run $number
  }

  proc getRun {} {
    variable run 
    return $run 
  }

  proc incrRun {} {
  }

  proc normalColors {} {}

  proc isRecording {} {}

  proc getTitle {} {return "Offline run"}

  proc Log {source class msg} {
    puts "$source $class $msg"
  }

  proc outputText {msg} {puts $msg}
}


# ---------------------------------------------------------------



# ---------------------------------------------------------------


snit::type OfflineEVBOutputPipeline {
  option -daqroot   -default {::InstallRoot::Where}

  variable fd -1
  variable running false

  constructor {args} {
    $self configurelist $args
  }

  destructor {
    if {[$self getPipeFD]>0} {
      catch {close [$self getPipeFD]} msg 
      $self setPipeFD -1
      $self setRunning false
    }
  }

  ## @brief Launch the pipeline using the parameters passed in
  #
  method launch params {
    if {[catch {ringbuffer usage [$params cget -ringname]} msg]} {
      # failed to find the ring
      ringbuffer create [$params cget -ringname]
    }

    set command [$self createPipelineCommand $params]
      
    $self setPipeFD [open $command r]
    set theFD [$self getPipeFD]
    # update the running state
    $self setRunning true

    chan configure $theFD -blocking 0
    chan configure $theFD -buffering line 
    chan event $theFD readable [mymethod _handleReadable $theFD]
  }


  method setPipeFD {newfd} {
    set fd $newfd
  }

  method getPipeFD {} {
    return $fd
  }

  method setRunning {onoff} {
    set running $onoff 
  }

  method getRunning {} {
    return $running 
  }

  ## @brief Form the command that will be executed
  #
  method createPipelineCommand params {
    set ringname  [$params cget -ringname]
    set prefix    [$params cget -prefix]
    set dir       [$params cget -stagearea]
    set nsources  [$params cget -nsources]

    set daqbin [file join [InstallRoot::Where] bin]
    set command    "| $daqbin/eventlog --source=$ringname "
    append command "--prefix=$prefix "
    append command "--path=$dir "
    append command "--oneshot --number-of-sources=$nsources "
    append command "|& cat"

    return $command
  }

  ## @brief Callback for when the file descriptor is readable
  #
  method _handleReadable {fid} {

    if {[catch {gets $fid line} len] || [chan eof $fid]} {
      puts "Input pipeline has exited"
      chan event $fid readable "" 
      catch {close $fid}
      $self setPipeFD   -1
      $self setRunning false
    } elseif {$len>0} {
      puts $line
    }
  }

}

#
#package require Tk
#
#set ::pipe ""
#ttk::button .start -text "Start" -command StartPipeline
#ttk::button .stop -text "Stop" -command { set pids [pid [$::pipe getPipeFD]]; foreach pid $pids { exec kill $pid} }
#
#grid .start
#grid .stop
#
#proc StartPipeline {} {
#  set ::pipe [OfflineEVBOutputPipeline %AUTO%]
#  set params [OfflineEVBOutputPipeParams %AUTO%]
#  $::pipe launch $params
#}
#

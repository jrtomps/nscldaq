
package provide OfflineEVBOutputPipeline 11.0

package require snit
package require InstallRoot 
package require ring
package require Configuration
package require eventLogBundle

snit::type OfflineEVBOutputPipeParams {
  option -ringname     -default "OfflineEVBOut"
  option -prefix       -default "reordered"
  option -stagearea    -default [file join $::env(HOME) offlinestagearea]
  option -nsources     -default "2"
  option -logger       -default {[Configuration::get EventLogger]}
  option -usensrcs     -default 1
  option -forcerun     -default 0
  option -usechecksum  -default 1 

  constructor {args} {
    $self configurelist $args   
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
    set command    "| $daqbin/eventlog --source=tcp://localhost/$ringname "
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

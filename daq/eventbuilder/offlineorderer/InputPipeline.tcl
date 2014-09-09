

package provide OfflineEVBInputPipeline 11.0
package require snit 
package require InstallRoot 
package require ring


snit::type OfflineEVBInputPipeParams {

  option -file      -default ""
  option -unglomid  -default 0
  option -inputring -default "OfflineEVBIn"

  constructor {args} {
    $self configurelist $args
  }
}



snit::type OfflineEVBInputPipeline {
  
  option -daqroot -default {::InstallRoot::Where}  -validatemethod SetPath


  variable running    false
  variable fd         -1 

  constructor {args} {
    $self configurelist $args
  }

  destructor {
    if {$fd > 0} {
      catch "close $fd" msg
    }
  }

  typemethod validateParameters {params context} {
     set keys [$params info options]
     if {"-unglomid" ni $keys} {
       set msg "$context : no source id specified for unglom"
       return -code error $msg 
     }

     if {"-inputring" ni $keys} {
       set msg "$context : no ring specified for input pipeline"
       return -code error $msg
     }

     if {[$params cget -file] eq ""} {
       set msg "$context : no input file specified for input pipeline"
       return -code error $msg
     } else {
       set path [$params cget -file]
       if {![file exists $path]} {
         set msg    "$context : input file \"$path\" specified for input "
         append msg "pipeline but does not exist"
         return -code error $msg
       }
     }

     # if none of the above conditions returned early, the
     # parameters are reasonable
  }

  ## @brief ensure that a path exists before setting it

  method SetPath {option value} {
    if {![file exists $value]} {
      return -code error "OfflineEVBInputPipeline::SetPath passed path that does not exist for option: $option"
    } 
  }

  method launch {params} {
    if {[catch {ringbuffer usage [$params cget -inputring]} msg]} {
      # failed to find the ring
      ringbuffer create [$params cget -inputring]
    }

    # Check whether or not the params contain good infomation
    OfflineEVBInputPipeline validateParameters $params "OfflineEVBInputPipeline::launch"

    # Set up the command to run based on the namespace variables
    set cmd [$self _createPipelineCommand $params]

    puts $cmd

    # launch the pipeline 
    $self setPipeFD [open $cmd r]
    # update the running state
    $self setRunning true

    set theFD [$self getPipeFD]
    chan configure $theFD -blocking 0
    chan configure $theFD -buffering line 
    chan event $theFD readable [mymethod _handleReadable]

  }

  method _createPipelineCommand {params} {

    set unglomid  [$params cget -unglomid]
    set inputring [$params cget -inputring]
    set fnames     [$params cget -file]

    set daqbin [file join [InstallRoot::Where] bin]
    set pipeline     "| cat "
    foreach file $fnames {
      append pipeline "$fnames "
    }
    append pipeline  "| $daqbin/unglom --id $unglomid "
    append pipeline  "| $daqbin/frag2ring --strip "
    append pipeline  "| $daqbin/stdintoring $inputring "
    append pipeline  "|& cat"

    return $pipeline
  }

  method getPipeFD {} {
    return $fd
  }

  method setPipeFD {newfd} {
    set fd $newfd
  }

  method setRunning {yesno} {
    set running $yesno
  }

  method getRunning {} {
    return $running
  }



  method _handleReadable {} {
    set theFD [$self getPipeFD]
    set line [read $theFD]
    if {[eof $theFD]} {
      puts "Input pipeline has exited"
      chan event $theFD readable {}
      catch {close $theFD} msg

      $self setPipeFD -1
      $self setRunning false
    } else {
      puts $line
    }
  }
}




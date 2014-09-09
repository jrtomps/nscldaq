#!/usr/bin/env tclsh


package provide OfflineEVBRunProcessor 11.0

package require snit
package require OfflineEVBJobProcessor
package require OfflineEVBOutputPipeline
package require evbcallouts
package require RunstateMachine 


snit::type RunProcessor {

  option -jobs 
  
  component processor

  ## @brief Pass the options
  #
  constructor {args} {
    $self configurelist $args

    set processor [JobProcessor %AUTO% -runprocessor $self]

  }

  destructor {
    $processor destroy
  }


  ## @brief Entry point for the actual processing
  # 
  # Sets up all the pipelines and then processes all of the files provided
  #
  method run {} {

    $self runNext

  }

  method runNext {} {

    # get the list of jobs
    set jobs [dict keys $options(-jobs)]

    # if we have a job still, then configure it and fire it off
    if {[llength $jobs]>0} {

      # get the first job name
      set job [lindex $jobs 0]


      # copy the job 
      set iparams [dict get $options(-jobs) $job -inputparams]
      if {[catch {set run [$self guessRunNumber [$iparams cget -file]]} msg]} {
        return -code error "RunProcessor::run failed to identify the run number"
      }
      ReadoutGUIPanel::setRun $run 
      puts "$job , run $run"
      $processor configure -inputparams $iparams 
      $processor configure -hoistparams [dict get $options(-jobs) $job -hoistparams]
      $processor configure -evbparams   [dict get $options(-jobs) $job -evbparams]
      $processor configure -outputparams [dict get $options(-jobs) $job -outputparams]

      # launch this thing
      $processor run

      # remove the current processing job
      set options(-jobs) [dict remove $options(-jobs) $job]
    }
  }

  method guessRunNumber {files} {
    set file [file tail [lindex $files 0]]
    set pattern {^(\w+)-(\d+)-(\d+).evt$}
    if {[regexp $pattern $file match prefix run segment]} {
      # the number is treated as octal if it is padded with 0 on the left
      # so we need to trim those off
      set run [string trimleft $run "0"]
      return $run 
    } else {
      return -code error "RunProcessor::guessRunNumber unable to parse run number from file list"
    }
  }

}


namespace eval EVBManager {

  ## No-op.
  #
  # We want to configure this elsewhere so we will pass
  #
  proc attach {state} {
  }

  ##
  #
  proc enter {from to} {
    if {($from in [list Active Paused]) && ($to eq "Halted")} {
      EVBC::onEnd
      EVBC::stop
    } 

    if {($from in [list Active Paused]) && ($to eq "NotReady")} {
      EVBC::onEnd
      EVBC::stop
    } 

  }


  proc leave {from to} {

    if {($from eq "Halted") && ($to eq "Active")} {
      EVBC::onBegin
    }

  }

  proc register {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle EVBManager
    $sm destroy
  }

  namespace export attach enter leave

}


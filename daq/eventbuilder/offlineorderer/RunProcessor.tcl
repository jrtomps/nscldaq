#!/usr/bin/env tclsh


package provide OfflineEVBRunProcessor 11.0

package require snit
package require OfflineEVBJobProcessor
package require OfflineEVBOutputPipeline
package require evbcallouts
package require RunstateMachine 


snit::type RunProcessor {

  option -jobs 

  variable m_runObservers
  
  component processor

  ## @brief Pass the options
  #
  constructor {args} {
    variable m_runObservers

    $self configurelist $args

    set processor [JobProcessor %AUTO% -runprocessor $self]

    set m_runObservers [list]
  }

  destructor {
    $processor destroy
  }


  ## @brief Entry point for the actual processing
  # 
  # Sets up all the pipelines and then processes all of the files provided
  #
  method run {} {

    $self observeNewRun 

    $self runNext

  }

  method runNext {} {

    # get the list of jobs
    set jobs [dict keys $options(-jobs)]

    # if we have a job still, then configure it and fire it off
    if {[llength $jobs]>0} {

      # get the first job name
      set job [lindex $jobs 0]

      $self observeNewJob $job

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

      # launch this thing but stop if it was aborted.
      if {[catch {$processor run} msg]} {
        tk_messageBox -icon error -message $msg
        $self observeAbort
        return
      }

      # remove the current processing job
      set options(-jobs) [dict remove $options(-jobs) $job]
    } else {
      $self observeCompleted
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
  
  method addRunStatusObserver {observer} {
    variable m_runObservers
    lappend m_runObservers $observer
  }

  method observeNewRun {} {

    set newdict [dict create queued     [dict keys $options(-jobs)] \
                             processing "" \
                             completed  [list]]

    foreach observer $m_runObservers {
      $observer setModel $newdict
    }
  }

  method observeNewJob {jobName} {
    puts "observeNewJob"
    foreach observer $m_runObservers {
      $observer transitionToNextJob $jobName
    }
  }

  method observeCompleted {} {
    puts "observeCompleted"
    foreach observer $m_runObservers {
      $observer finish 
    }
  }

  method observeAbort {} {
    puts "observeAbort"
    foreach observer $m_runObservers {
      $observer finish 
    }

  }
}


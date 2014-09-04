#!/usr/bin/env tclsh


package provide OfflineOrderer 11.0
package require snit
package require OfflineEVBInputPipeline 
package require OfflineEVBHoistPipeline 
package require evbcallouts
package require OfflineEVBOutputPipeline 


snit::type OfflineOrdererProcessor {

  option -files        -default ""
  option -inputparams  -default ""
  option -hoistparams  -default ""
  option -evbparams    -default ""
  option -outputparams -default ""


  variable processingStatus ;#< Status of processing
  variable inputPipeline  "" 
  variable outputPipeline ""

  ## @brief Pass the options
  #
  constructor {args} {
    $self configurelist $args

    set processingStatus [dict create queued [list $options(-files)] completed [list] current ""]
  }


  ## @brief Entry point for the actual processing
  # 
  # Sets up all the pipelines and then processes all of the files provided
  #
  method run {} {
    # hook to handle one-time startup procedures 
    $self setup

    # process each file that has been provided
    foreach f $options(-files) { 
      $self processFile $f
    }

    # hook to handle one-time teardown procedures 
    $self tearDown

  }

  ## @brief Hook for one-time startup procedures 
  #
  #
  method setup {} {
    $self launchEVBPipeline

    # wait until the event builder has started up and is accepting connections 
    ::EVBC::_waitForEventBuilder

    $self launchHoistPipeline
  }

  ## @brief Hook for one-time startup procedures 
  #
  #
  method tearDown {} {
    EVBC::onEnd 
  }



  ## @brief Launch pipelines that live for the duration of a file
  method processFile {fname} {

    # set the file list 
    $options(-inputparams) configure -file $fname

    # 2. Start up the logger
    $self launchOutputPipeline

    # 4. Send the input 
    $self launchInputPipeline

  }


  ## @brief Create the ringFragmentSource
  #
  method launchHoistPipeline {} {
    
    set ringurl   "tcp://localhost/[$options(-hoistparams) cget -sourcering]"
    set tstamplib [$options(-hoistparams) cget -tstamplib]
    set id        [$options(-hoistparams) cget -id]
    set info      [$options(-hoistparams) cget -info]
    ::EVBC::startRingSource $ringurl $tstamplib $id $info
  }

  ## @brief Launch the output pipeline
  #
  method launchOutputPipeline {} {
    set output [OfflineEVBOutputPipeline %AUTO%]
    $output launch $options(-outputparams)
    set outputPipeline $output
  }

  ## @brief 
  #
  method launchInputPipeline {} {
    set input [OfflineEVBInputPipeline %AUTO%]
    $input launch $options(-inputparams)
    set inputPipeline $input
  }


  method launchEVBPipeline {} {
    $self initializeEVB 
    EVBC::onBegin 
  }

  ## @brief Set parameters in the event builder
  # @param params   an AppOptions object
  method initializeEVB {} {

    # set up a convenience name for options(-evbparams) 
    set params $options(-evbparams)

    # -evbparams is unique b/c the options have already been encapsulated as
    # EVBC::AppOptions. We therefore use those by default.
    set res [catch {
      expr {[$params info type] eq "EVBC::AppOptions"}
    } msg]


    if {$res} {
      return -code error $msg
    } else {
      $params configure -gui false
      $params configure -restart false

      set opts [$params info options]
      foreach opt $opts {
        dict set optDict $opt [$params cget $opt]
      }

      EVBC::initialize {*}$optDict
    }

     
  }


}



#proc EVBC::_Output msg {
#  puts $msg
#}
#

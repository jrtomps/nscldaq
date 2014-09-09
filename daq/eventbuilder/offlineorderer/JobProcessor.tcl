
package provide OfflineEVBJobProcessor 11.0

package require snit
package require eventLogBundle
package require OfflineEVBInputPipeline
package require evbcallouts
package require OfflineEVBOutputPipeline
package require DataSourceManager
package require DataSourceMonitor
package require ExpFileSystem
package require rdoCalloutsBundle

namespace eval HoistConfig {
  variable tstamplib
  variable info
  variable ring
  variable id
}

snit::type JobProcessor {

  option -inputparams  -default ""
  option -hoistparams  -default ""
  option -evbparams    -default ""
  option -outputparams -default ""

  variable sourceManager ""
  variable stateMachine  ""

  ## @brief Pass the options
  #
  constructor {args} {
    variable sourceManager
    variable stateMachine
    set sourceManager [DataSourcemanagerSingleton %AUTO%]
    set stateMachine  [RunstateMachineSingleton %AUTO%]

    $self configurelist $args

  }

  method setupStateMachine {} {
    ::EVBManager::register
    ::EventLog::register
    ::DataSourceMgr::register
    ::DataSourceMonitor::register

  }

  method getDataSourceManager {} {
    variable sourceManager
    return $sourceManager

  }

  ## @brief Entry point for the actual processing
  # 
  # Sets up all the pipelines and then processes all of the files provided
  #
  method run {} {
    # hook to handle one-time startup procedures 
    puts "setup"
    $self setup

    puts "startProcessing"
    $self startProcessing 
  }

  ## @brief Hook for one-time startup procedures 
  #
  #
  method setup {} {
    puts "setupStateMachine"
    $self setupStateMachine 

    puts "generateStartEVBSources"
    $self generateStartEVBSources 

    puts "configureEVB"
    $self configureEVB

    puts "configureEventLog"
    $self configureEventLog

    puts "setupDataSourceManager"
    $self setupDataSourceManager

  }

  ## @brief Launch pipelines that live for the duration of a file
  method startProcessing {} {
    variable stateMachine

    # Transition the state machine 
    puts "NotReady -> Starting"
    $stateMachine transition Starting
#    puts "Starting -> Halted"
    $self waitForHalted
#    $stateMachine transition Halted 
    puts "Halted -> Active"
    $stateMachine transition Active 

  }

  method waitForHalted {} {
    variable stateMachine

    set state [$stateMachine getState]
    puts $state
    while {$state ne "Halted"} {
      after 100 
      update
      set state [$stateMachine getState]
      puts $state
    }
  }
 
  ##
  #
  method stopProcessing {} {
    variable stateMachine
    $stateMachine transition Halted
    $stateMachine transition NotReady
  }

  ## Use the hoist parameters to construct the startEVBSources method
  #
  # THIS IS DIRTY AND COULD POTENTIALLY BE REPLACED WITH A hoist pipeline callout bundle
  method generateStartEVBSources {} {
    set ::HoistConfig::tstamplib [$options(-hoistparams) cget -tstamplib]
    set ::HoistConfig::info  [$options(-hoistparams) cget -info]
    set ::HoistConfig::ring  [$options(-hoistparams) cget -sourcering]
    set ::HoistConfig::id    [$options(-hoistparams) cget -id]

    # define a startEVBSources proc or overwrite it if it already exists
    eval { proc ::startEVBSources {} { EVBC::startRingSource tcp://localhost/$::HoistConfig::ring $::HoistConfig::tstamplib $::HoistConfig::id $::HoistConfig::info }}
  }

  ## Pass the configuration information to the ::EVBC:: parameters
  #
  method configureEVB {} {

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

  ## Pass the configuration info to the event log
  #
  method configureEventLog {} {
    set stagearea [$options(-outputparams) cget -stagearea]
    Configuration::Set StageArea  $stagearea
    if {! [file exists [ExpFileSystem::getStageArea]]} {
      ExpFileSystem::CreateHierarchy
    }

    # set up the parameters
    Configuration::Set EventLogRunFilePrefix     [$options(-outputparams) cget -prefix]
    Configuration::Set EventLoggerRing [$options(-outputparams) cget -ringname]
    Configuration::Set EventLogUseNsrcsFlag      1 

    # when starting the eventlog the builder will compute how many data sources are 
    # registered and uses that number to set the value of the --number-of-sources
    # parameter. Since we are only expecting 1 data source, then we can set up the
    Configuration::Set EventLogAdditionalSources [expr [$options(-outputparams) cget -nsources]-1] 
    Configuration::Set EventLogUseChecksumFlag   1 
  }

  ##  Make sure that each time we start up a new job,
  #   the data sources are all started from a stopped
  #   state. 
  #
  method setupDataSourceManager {} {
    variable sourceManager
    $sourceManager load Offline
    set sources [$sourceManager sources]
    foreach source $sources {
      set id [dict get $source sourceid]
      $sourceManager removeSource $id
    }

    $sourceManager addSource Offline [dict create unglomid [$options(-inputparams) cget -unglomid] \
                                                ring [$options(-inputparams) cget -inputring] \
                                                file [$options(-inputparams) cget -file]]
  }

#  ## @brief Create the ringFragmentSource
#  #
#  method launchHoistPipeline {} {
#    
#    set ringurl   "tcp://localhost/[$options(-hoistparams) cget -sourcering]"
#    set tstamplib [$options(-hoistparams) cget -tstamplib]
#    set id        [$options(-hoistparams) cget -id]
#    set info      [$options(-hoistparams) cget -info]
#
#    ::EVBC::startRingSource $ringurl $tstamplib $id $info
#  }


}



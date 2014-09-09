
package provide OfflineEVBJobProcessor 11.0

package require snit
package require eventLogBundle

package require DataSourceManager
package require DataSourceMonitor
package require EventLogMonitor

package require ExpFileSystem
package require rdoCalloutsBundle
package require OfflineEVBOutputPipeline
package require OfflineEVBInputPipeline
package require evbcallouts

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

  option -runprocessor -default ""

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

  ## @brief Register all of the bundles that we need
  # and no more
  #
  method setupStateMachine {} {
    # rdoCalloutsBundle has already been registered
    ::EVBManager::register
    ::EventLog::register
    ::DataSourceMgr::register
    ::DataSourceMonitor::register

    set ::EventLogMonitor::fdvar  ::EventLog::loggerPid
    set ::EventLogMonitor::script [list [mymethod stopProcessing]]
    ::EventLogMonitor::register
  }

  ## @brief return the data source manager known to this
  #
  method getDataSourceManager {} {
    variable sourceManager
    return $sourceManager

  }

  ## @brief return the state machine known to this
  #
  method getRunStateMachine {} {
    variable stateMachine 
    return $stateMachine
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
  #
  # This is actually very simple because it merely transitions the
  # state machine into an Active state and tells the EventLog that 
  # it is okay for it to exit without an error.
  #
  method startProcessing {} {
    variable stateMachine

    # Transition the state machine to Starting (note this schedules
    # a transition to Active on its own)
    $stateMachine transition Starting

    # We need to wait for the scheduled transition to succeed
    # before transitioning to Active because otherwise it will fail
    $self waitForHalted

    $stateMachine transition Active 

    # Tell the eventlog that it is okay for it to exit, because that is 
    # what we expect.
#    set EventLog::expectingExit 1

    EventLog::runEnding
  }

  ## @brief Iteratively check to see if the state machine has become Halted.
  #
  # This polls every 100 ms.
  #
  # During each iteration, the window should remain live because of
  # the update calls.
  #
  method waitForHalted {} {
    variable stateMachine

    set state [$stateMachine getState]
    while {$state ne "Halted"} {
      after 100 
      update
      set state [$stateMachine getState]
    }
  }
 
  ## @brief Transition the state machine into a NotReady state
  #
  method stopProcessing {} {
    puts "stopProcessing"
    variable stateMachine

    if {[$stateMachine getState] eq "Active"} {
      puts "Active -> Halted"
      $stateMachine transition Halted
    }

    puts "Halted -> NotReady"
    $stateMachine transition NotReady

    $self tearDown 

    $options(-runprocessor) runNext
  }

  ## @brief Transition the system into a clean state
  #
  method tearDown {} {

    puts "clearDataSources"
    $self clearDataSources
    
    puts "tearDownStateMachine"
    $self tearDownStateMachine
  }


  ## @brief Remove all of the data providers 
  #
  # There really should only be one data provider in the
  # DataSourceManager: Offline. However, we will treat the
  # scenario that there are lots in case someone down the 
  # road decides we need more.
  #
  method clearDataSources {} {
    variable sourceManager

    # unload the Offline provider... catch it because it can fail 
    catch {$sourceManager unload Offline}

    # Remove all of the data sources
    set sources [$sourceManager sources]
    foreach source $sources {
      set id [dict get $source sourceid]
      $sourceManager removeSource $id
    }


  }

  ## @brief Remove all callout bundles from the state machine
  #
  method tearDownStateMachine {} {
    variable stateMachine

    set bundles [$stateMachine listCalloutBundles]
    foreach bundle $bundles {
      $stateMachine removeCalloutBundle $bundle
    }

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

}



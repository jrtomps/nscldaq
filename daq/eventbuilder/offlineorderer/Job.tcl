
package provide OfflineEVBJob 11.0

package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require OfflineEVBOutputPipeline
package require evbcallouts

namespace eval Job {


  proc create {} {
    set iparams [OfflineEVBInputPipeParams %AUTO%]
    set hparams [OfflineEVBHoistPipeParams %AUTO%]
    set eparams [EVBC::AppOptions %AUTO%]
    set oparams [OfflineEVBOutputPipeParams %AUTO%]

    # Create the job parameters  to pass to the configuration
    # dialogue. It becomes the model that the dialogue's presenter
    # manipulates.
    set model [dict create  -jobname "Job" \
                            -inputparams $iparams  \
                            -hoistparams $hparams \
                            -evbparams   $eparams \
                            -outputparams $oparams ]
    return $model
  }


  ##
  #
  #
  proc clone {source} {
    set iparams [[dict get $source -inputparams] clone]
    set hparams [[dict get $source -hoistparams] clone]
    set eparams [[dict get $source -evbparams] clone]
    set oparams [[dict get $source -outputparams] clone]

    set name [dict get $source -jobname]

    return [dict create -jobname $name \
								        -inputparams  $iparams \
								        -hoistparams  $hparams \
								        -evbparams    $eparams \
								        -outputparams $oparams]
  }


  proc destroy {source} {
    dict for {key val} $source {
      if {$key ne "-jobname"} {
        $val destroy
      }
    }
  }


  proc pickle {snitobj} {
    set vallist [dict create]
    set opts [$snitobj info options]

    foreach opt $opts {
      dict set vallist $opt [$snitobj cget $opt]
    }

    return $vallist
  }

}

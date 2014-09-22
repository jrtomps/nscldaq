
package provide OfflineEVBJob 11.0

namespace eval Job {

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

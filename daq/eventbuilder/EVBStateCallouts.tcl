
package provide EVBStateManager 11.0

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
    } 

    if {($from in [list Active Paused Halted]) && ($to eq "NotReady")} {
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


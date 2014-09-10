

package provide EventLogMonitor 11.0

namespace eval EventLogMonitor {
  variable fdvar
  variable script

  proc onExit {name1 name2 op} {
    variable fdvar
    variable script 

    set fd [set $fdvar]
    puts "EventLogMonitor::onExit fd=$fd"
    if {($fd == -1) && ([string length $script]!=0)} {

      uplevel #0 eval $script

      trace remove variable $fdvar write ::EventLogMonitor::onExit
      puts [trace info variable $fdvar]
    }
  }

  ## Register the trace if NotReady
  #
  proc attach {state} {
    if {$state eq "NotReady"} {
      variable fdvar
      trace add variable $fdvar write ::EventLogMonitor::onExit
    }
  }

  ## Register the trace when entering Active state
  #
  proc enter {from to} {
    variable fdvar

    if {($from eq "Halted") && ($to eq "Active")} {
      variable fdvar
      puts "trace add variable $fdvar write ::EventLogMonitor::onExit"
      puts "$fdvar : $::EventLogMonitor::script"
#      if {[llength [trace info variable $fdvar]]==0} {
        trace add variable $fdvar write ::EventLogMonitor::onExit
#      }
    }
  }

  ## Remove the trace when leaving Active state
  proc leave {from to} {
    variable fdvar

    if {($from eq "Active") && ($to in [list "Halted" "NotReady"])} {
      puts "trace remove variable $fdvar write ::EventLogMonitor::onExit"
      trace remove variable $fdvar write ::EventLogMonitor::onExit
      puts [trace info variable $fdvar]
    }

  }

  proc register {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle EventLogMonitor
    $sm destroy
  }

  namespace export attach enter leave
}

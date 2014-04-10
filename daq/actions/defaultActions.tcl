
package provide DefaultActions 1.0
package require ReadoutGUIPanel

namespace eval DefaultActions {

  proc handleError str { 
    set trimmedstr [string trimright $str " \n"]
      ReadoutGuiPanel::Log error $trimmedstr
  }

  proc handleLog {str} {
    set trimmedstr [string trimright $str " \n"]
    ReadoutGuiPanel::Log log $trimmedstr
  }

  proc handleWarning {str} {
    set trimmedstr [string trimright $str " \n"]
    ReadoutGuiPanel::Log warning $trimmedstr
  }

  proc handleDebug {str} {
    set trimmedstr [string trimright $str " \n"]
    ReadoutGuiPanel::Log debug $trimmedstr
  }

  proc handleOutput {str} {
    set trimmedstr [string trimright $str " \n"]
    ReadoutGuiPanel::Log output $trimmedstr
  }

  proc handleTclCommand {str} {
    set trimmedstr [string trimright $str " \n"]
    eval $trimmedstr
  }

  namespace export handleError handleLog \
            handleWarning handleDebug \
            handleOutput handleTclCommand
  namespace ensemble create
}


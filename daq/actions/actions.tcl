
package provide Actions 1.0

namespace eval Actions {
  variable line
  variable errors [dict create 0 "no directive provided"]
  variable directiveMap { ERRMSG 0 LOGMSG 1 WRNMSG 2 
                          TCLCMD 3 OUTPUT 4 DBGMSG 5} 

  variable actionBundle DefaultActions

  proc onReadable {fd} {
    variable line 
    variable errors 

    if { [eof $fd] } {
      chan event $fd readable ""
      catch {close $fd} msg
    } else {

      set input [read $fd ]
      if {[string length $input] > 0} {
        append data $line $input
        if {[string first "\n" $data] != -1} {
          append $line $data

          catch {set parsedLine [parseDirective $line]} msg
          if {$msg eq [dict get $errors 0]} {
            return -code error $msg
          } else {
            handleMessage $parsedLine
          }
          # reset our line to a null string
          set $line ""
        }
      
      }; # end of nonzero input
    }
  }

  proc parseDirective {full_line} {
    variable errors
    # directives have a form "DIRECTIVE message"
    # so we just need to find the first space

    set parsedLine [list]

    set index [string first { } $full_line]
    if { $index != -1 } {
      set dir [string range $full_line 0 [expr $index-1]] 
      set msg [string range $full_line [expr $index+1] end]
      lappend parsedLine $dir 
      lappend parsedLine $msg
    } else {
      return -code error [dict get $errors 0]
    }

    return $parsedLine
  }

  proc handleMessage {parsedLine} {
    variable directiveMap

    set directive [lindex $parsedLine 0]
    set msg [lindex $parsedLine 1]
    set dirId [dict get $directiveMap $directive]
    switch $dirId {
      0 { handleError $msg }
      1 { handleLog $msg }
      2 { handleWarning $msg }
      3 { handleTclCommand $msg }
      4 { handleOutput $msg }
      5 { handleDebug $msg }
    } 
  }

  proc handleError {str} {
     variable actionBundle
     $actionBundle handleError $str
  }

  proc handleLog {str} {
    variable actionBundle
    $actionBundle handleLog $str
  }

  proc handleWarning {str} {
    variable actionBundle
    $actionBundle handleWarning $str
  }

  proc handleDebug {str} {
    variable actionBundle
    $actionBundle handleDebug $str
  }

  proc handleOutput {str} {
    variable actionBundle
    $actionBundle handleOutput $str
  }

  proc handleTclCommand {str} {
    variable actionBundle
    $actionBundle handleTclCommand $str
  }
}


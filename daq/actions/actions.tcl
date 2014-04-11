# actions.tcl
#
# A tcl package to handle the parsing of a basic messaging protocol
# to allow communication between devices in a pipeline and the ReadoutGUI.
#
# Things that need to be fixed. 
#   - Need to fix the leaking of the process.
#   - sometimes a newline is prepended to an output and that needs to be 
#     fixed.
#


package provide Actions 1.0


namespace eval Actions {
  
  variable line "" 
  variable incomplete 0 
  variable errors [dict create 0 " unable to parse directive"]
  variable legalDirectives {ERRMSG LOGMSG WRNMSG TCLCMD OUTPUT DBGMSG}
  variable directiveMap { ERRMSG 0 LOGMSG 1 WRNMSG 2 
                          TCLCMD 3 OUTPUT 4 DBGMSG 5} 

  variable actionBundle DefaultActions

  proc onReadable {fd} {
    variable incomplete 0
    
    if { [eof $fd] } {
      # unregister itself
      chan event $fd readable ""
      catch {close $fd} msg
      puts "End of file reached"
    } else {
      handleReadable $fd 
    }
  }

  proc handleReadable {fd} {
    variable line
    variable incomplete 

    # read what the channel has to give us
    set input [chan read $fd ]

    append line "$input"
    set line [string trimright $line "\n"]

    while {[string length $line]>0 && !($incomplete)} {

      set firstWord [extractFirstWord $line]

      # if we have a legal directive, treat it
      # as a packet
      if {[isLegalDirective $firstWord]} {
        set parsedLine [buildPacket ]
        if {!("$parsedLine" eq "")} {
          handleMessage $parsedLine
          set incomplete 0
        } 
      } else {
        handleNonPacket
      }
    }; # end of nonzero input
  }
 
  proc extractFirstWord {sentence} {
    return [string range $sentence 0 5]
  }

  # The first word was detected as a legal directive so 
  # we expect that there is a well formed packet. Try to
  # read the whole thing. If the full packet isn't there,
  # return a null string and move on. 
  # If the packet is found, truncate "line" so that the 
  # packet is no longer being outputted.
  proc buildPacket {} {
    variable line
    variable incomplete

    set incomplete 1

    # find first and second word boundaries 
    set b1 [string first { } $line 0]
    if {$b1 == -1} return
    set b2 [string first { } $line [expr $b1+1]]
    if {$b2 == -1} return
    
    set pktSize [string trim [string range $line $b1 $b2] { \n}]
    set totalLength [string length $line]
    set remChars [expr $totalLength - ($b2+1)]

    if {$remChars >= $pktSize} {
       set b3 [expr $b2+$pktSize]
       lappend parsedLine [extractFirstWord $line] 
       lappend parsedLine [string range $line [expr $b2+1] $b3] 
       lappend parsedLine [string range $line [expr $b1+1] [expr $b2-1]] 

       set incomplete 0
       set line [string range $line [expr $b3+1] end] 

       return $parsedLine
    } else {
       return ""
    }

  }

  # Deal with non packet output... we simply output
  # everything we have up until a valid directive is 
  # found 
  # if we find a directive, output everything up to that
  # directive, pop the outputted msg from the front of line,
  # return.

  proc handleNonPacket {} {
    variable line
    variable legalDirectives
  
    # Check if line contains any legal directives
    foreach dir $legalDirectives {
      set index [string first $dir $line]
      if {$index != -1} {
        set msg [string range $line 0 [expr $index-1]]
        set line [string range $line $index end]

        handleOutput $msg
        return ""
      }
    }

    # if we are here then we didn't find any directives
    handleOutput $line
    set line ""
  } 

  # Determine if there word is a legal directive
  proc isLegalDirective {word} {
    variable legalDirectives
    return [expr {$word in $legalDirectives}]
  }

  # Jump-table of sorts for passing various 
  # handlers to their handlers
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


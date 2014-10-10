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
package require snit


snit::type Actions {
  
  variable line "" 
  variable incomplete 0 
  variable errors [dict create 0 " unable to parse directive"]
  variable legalDirectives {ERRMSG LOGMSG WRNMSG TCLCMD OUTPUT DBGMSG}
  variable directiveMap { ERRMSG 0 LOGMSG 1 WRNMSG 2 
                          TCLCMD 3 OUTPUT 4 DBGMSG 5} 

  option -actionbundle -default DefaultActions

  constructor {args} {
    $self configurelist $args
  }

  method onReadable {fd} {
    set incomplete 0 

    if { [eof $fd] } {
      # unregister itself
      chan event $fd readable ""

      # we have reached an end of file
      #
      # convert to blocking to retrieve the exit status
       if {0} {
      chan configure $fd -blocking 1

      if {[catch {close $fd} msg]} {
        if {[lindex $::errorCode 0] eq "CHILDSTATUS"} {
          puts stderr "Child process exited abnormally with status: $::errorCode"
        }
      }
       }
    } else {
      $self handleReadable $fd 
    }
  }

  method getLine {} { return $line }
  method setLine {str} { set line $str }

  method handleReadable {fd} {

    # read what the channel has to give us
    set input [chan read $fd ]

    append line "$input"
    set line [string trimright $line " \0\n"]

    while {[string length $line]>0 && !($incomplete)} {
      set line [string trim $line]
      set firstWord [$self extractFirstWord $line]

      # if we have a legal directive, treat it
      # as a packet
      if {[$self isLegalDirective $firstWord]} {
        set parsedLine [$self buildPacket ]
        if {!("$parsedLine" eq "")} {
          $self handleMessage $parsedLine
          set incomplete 0
        } 
      } else {
        $self handleNonPacket
      }
    }; # end of nonzero input
  }
 
  method extractFirstWord {sentence} {
    return [string range $sentence 0 5]
  }

  # The first word was detected as a legal directive so 
  # we expect that there is a well formed packet. Try to
  # read the whole thing. If the full packet isn't there,
  # return a null string and move on. 
  # If the packet is found, truncate "line" so that the 
  # packet is no longer being outputted.
  method buildPacket {} {

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
       lappend parsedLine [$self extractFirstWord $line] 
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

  method handleNonPacket {} {
  
    # Check if line contains any legal directives
    foreach dir $legalDirectives {
      set index [string first $dir $line]
      if {$index != -1} {
        set msg [string range $line 0 [expr $index-1]]
        set line [string range $line $index end]

        $self handleOutput $msg
        return ""
      }
    }

    # if we are here then we didn't find any directives
    $self handleOutput $line
    set line ""
  } 

  # Determine if there word is a legal directive
  method isLegalDirective {word} {
    return [expr {$word in $legalDirectives}]
  }

  # Jump-table of sorts for passing various 
  # handlers to their handlers
  method handleMessage {parsedLine} {

    set directive [lindex $parsedLine 0]
    set msg [lindex $parsedLine 1]
    set dirId [dict get $directiveMap $directive]
    switch $dirId {
      0 { $self handleError $msg }
      1 { $self handleLog $msg }
      2 { $self handleWarning $msg }
      3 { $self handleTclCommand $msg }
      4 { $self handleOutput $msg }
      5 { $self handleDebug $msg }
    } 
  }

  method handleError {str} {
     $options(-actionbundle) handleError $str
  }

  method handleLog {str} {
    $options(-actionbundle) handleLog $str
  }

  method handleWarning {str} {
    $options(-actionbundle) handleWarning $str
  }

  method handleDebug {str} {
    $options(-actionbundle) handleDebug $str
  }

  method handleOutput {str} {
    $options(-actionbundle) handleOutput $str
  }

  method handleTclCommand {str} {
    $options(-actionbundle) handleTclCommand $str
  }
}



package provide BlockCompleter 1.0
package require snit


snit::type BlockCompleter {

  option -left -default "{"
  option -right -default "}"

  variable _text 
  variable _nLeftDelimiters 
  variable _nRightDelimiters 

  constructor {args} {
    $self configurelist $args

    set _text {}
    set _nLeftDelimiters 0
    set _nRightDelimiters 0
  }

  method getText {} {
    return $_text
  }

  method appendLine {text} {
    $self updateDelimiterCounts $text
    append _text $text
  }

  method isComplete {} {
    return [expr $_nLeftDelimiters == $_nRightDelimiters]
  }

  method updateDelimiterCounts {text} {
    set length [string length $text]
    set index 0
    while {$index < $length} {
      set char [string index $text $index]
      if {$char eq [$self cget -left]} {
        incr _nLeftDelimiters
      } elseif {$char eq [$self cget -right]} {
        incr _nRightDelimiters
      }

      incr index
    }
  }

  method getDelimiterCounts {} {
    return [list $_nLeftDelimiters $_nRightDelimiters]
  }


  method Reset {} {
    set _text {}
    set _nLeftDelimiters 0
    set _nRightDelimiters 0
  }
}

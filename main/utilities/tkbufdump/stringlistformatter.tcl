#
#  Provides a formatter for buffers that contain lists of null terminated
#  strings.
#

package require snit

set here [file dirname [info script]]
source [file join $here itemdefs.tcl]


snit::type StringListFormatter {
    option -body
    option -main
    option -order
    option -type
    
    variable typeNames -array {}
    
    constructor args {
        $self configurelist $args
        
        array set typeNames [list                      \
            $::PACKET_TYPE  "Packet types"                  \
            $::MONITORED_VARIABLES "Monitored variables"    \
        ]

    }
    #
    #   Format the item.
    #   The item body structure is:
    #   +------------------------------------------+
    #   | uint32_t time offset                     |
    #   +------------------------------------------+
    #   |time_t    Timestamp (assumed 32 bits)     |
    #   +------------------------------------------+
    #   | uint32_t number of strings               |
    #   +------------------------------------------+
    #   |    Storage containing null terminated    |
    #   |     strings                              |
    #   +            ...                           +
    #
    method format {} {
        set b   $options(-body)
        set w   $options(-main)
        set t   $options(-type)
        set o   $options(-order)
        
        #  Get the time offset, timestamp (formatted) and number of strings
        
        set offset      [get32 $b 0 $o]
        set stamp       [get32 $b 2 $o]
        set count       [get32 $b 4 $o]
        set timestamp   [clock format $stamp]
        
        #  Output the heading:
        
        $w add "$typeNames($t) $timestamp  $offset seconds into the run:"
        
        # Figure out how many bytes there are remaining in the body and fetch
        # the body as a byte string.
        #
        set bodyLength    [llength $b]
        set stringLengthW [expr {$bodyLength - 6}]
        set stringLengthB [expr {$stringLengthW*2}]
        set bytes [getbytes $b 6 $stringLengthB]
        
        set stringNum 0
        set offset    0
        
        #  Assemble the strings and output them

    
        while {$offset < $stringLengthB} {
            set string [$self assembleString $bytes $offset]

            $w add "String [format %03d $stringNum]: '$string'"
            
            set offset [expr {$offset + [string length $string] + 1}]
            incr stringNum
        }
    }
    #-------------------------------------------------------------------------
    #
    #  Private methods:
    #
    
    #
    #  Assemble a string of bytes into an ascii string.
    #  assembly terminates on a null byte or at end of string.
    #
    method assembleString {bytes offset} {
        set result ""
        set l      [llength $bytes]
        while {$offset < $l} {
            
            set c [lindex $bytes $offset]
            if {$c == 0} {
                return $result
            }
            append result [format %c $c]
            incr offset
        }
        return $result
    }
    
}

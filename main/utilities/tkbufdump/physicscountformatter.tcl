
    #  The structure of the event count body is:
    #
    #     +---------------------------------------+
    #     | uint32_t  Time offset                 |
    #     +---------------------------------------+
    #     | time_t    absolute time               |
    #     +---------------------------------------+
    #     | uint64_t  count of events.            |
    #     +---------------------------------------+
    #

package require snit
set here [file dirname [info script]]
source [file join $here itemdefs.tcl]

#  Formatter for event counts
#
#  OPTIONS:
#    -body  - provides the event body as a list of numbers.
#    -main   - provides the main widget
#    -order  - 0 for little endian order, 1 for bit.
#    -type   - Type of the item.
#
#  METHODS:
#     format - Formats the buffer into the main widget.
#
snit::type PhysicsCountFormatter {
    option -body
    option -main
    option -order
    option -type
    
    constructor args {
        $self configurelist $args
    }
    #  Format the data:
    #  The structure of the event count body is:
    #
    #     +---------------------------------------+
    #     | uint32_t  Time offset                 |
    #     +---------------------------------------+
    #     | time_t    absolute time               |
    #     +---------------------------------------+
    #     | uint64_t  count of events.            |
    #     +---------------------------------------+
    #
    method format {} {
        set w $options(-main)
        set o $options(-order)
        set b $options(-body)
        
        # Get the 32 bit items:
        
        set offset [get32 $b 0 $o]
        set time   [get32 $b 2 $o]
        set ftime  [clock format $time]
        
        #  The count is 64 bits which are two 32 bit items.
        #  How these are assembled depends on the byte order
        
        if {$o} {
            # Big endian
            set h  [get32 $b 4 $o]
            set l  [get32 $b 6 $o]
        } else {
            # Little endian.
            
            set h  [get32 $b 6 $o]
            set l  [get32 $b 4 $o]
        }
        set ::tcl_precision 17;       # Double precision...
        set triggers [expr {1.0*$l + 4294967296.0*$h}]
        
        $w add "$ftime - Trigger counts at $offset seconds into the run [format %lf $triggers]"
    }
}


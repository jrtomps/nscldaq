package require snit

set here [file dirname [info script]]
source [file join $here itemdefs.tcl]

#
#   provide a snit formatter for control buffers
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
snit::type ControlFormatter {
    
    option  -body
    option  -main
    option  -order
    option  -type
    variable itemTypeNames
    #
    #  Construction just processes the configuration list.
    #
    constructor {args} {
        $self configurelist $args
        
        # Fill in itemTypeNames .. since we can't do substs in array -.
        
        
        array set  itemTypeNames [list  $::BEGIN_RUN "Begin Run"    \
                                        $::END_RUN   "End Run"      \
                                        $::PAUSE_RUN "Pause Run"    \
                                        $::RESUME_RUN "Resume Run"]
    }
    #
    #  Format the event.  The only thing that differs from
    #  item type to item type is the typename for the item.
    #
    method format {} {

        set w $options(-main)
        set o $options(-order)
        set type $itemTypeNames($options(-type))
        set body $options(-body)
        
        #  The structure of the data are:
        #  +--------------------------------------+
        #  | uint32_t  run Number                 |
        #  +--------------------------------------+
        #  | uint32_t seconds into run            |
        #  +--------------------------------------+
        #  | time_t stamp [seconds since epoch]   |
        #  +--------------------------------------+
        #  | title TITLE_MAXSIZE +1 bytes         |
        #  +--------------------------------------+
        #
        
        # Fetch run number and elapsed time:
        
        set run      [get32 $body 0 $o]
        set elapsed  [get32 $body 2 $o]
        
        # Fetch the timestamp:
        
        set tstamp  [get32 $body 4 $o]
        set fstamp  [clock format $tstamp]
        
        # Format all but the title:
        
        $w add "Control buffer $type at $fstamp"
        $w add "Run number $run  "
        $w add "Elapsed time $elapsed "
        
        #  Get the title
        
        set titlebytes [getbytes $body 6 $::TITLE_MAXSIZE]
        set titlestring ""
        for {set i 0} {$i < $::TITLE_MAXSIZE} {incr i} {
            set c [lindex $titlebytes $i]
            if {$c == 0} {
                break;                  # Null string.
            }
            append titlestring [format %c $c]
        }
        $w add "Title: '$titlestring'"
    }
}



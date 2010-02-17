#
#  Formats a scaler event.
#
package require Tk
package require snit

set here [file dirname [info script]]

source [file join $here itemdefs.tcl]
#
#  Formats a scaler event as a set of lines containing scaler increments
#  and rate information.
# OPTIONS:
#    -body            - Body of the item.
#    -main            - tkbufdump main megawidget.
#    -order           - Byte order 1 bigendian 0 little endian.
#    -type            - Type of item ($::INCREMENTAL_SCALERS)
# METHODS:
#    format           - Writes the formatted information into the
#                       tkbufdump main megawidget.
#
snit::type ScalerFormatter {
    option -body
    option -main
    option -order
    option -type
    #
    #  Formats the data into the main megawidget.  The body has the following
    #  format:
    #
    #  +-------------------------------+
    #  | uint32_t startTime            |
    #  +-------------------------------+
    #  | uint32_t endTime              |
    #  +-------------------------------+
    #  | time_t absolute time          |
    #  +-------------------------------+
    #  | uint32_t scaler count         |
    #  +-------------------------------+
    #  |   incremental scalers         |
    #         ...
    #
    method format {} {
        set b   $options(-body)
        set w   $options(-main)
        set o   $options(-order)
        
        #  Pull the interval start time and end time and
        #  compute the interval duration:
        #
        set start       [get32 $b 0 $o]
        set end         [get32 $b 2 $o]
        set duration    [expr {$end-$start}]
        
        # Get and format the timestamp which is assumed to be 32 bits:
        #
        
        set stamp       [get32 $b 4 $o]
        set fstamp      [clock format $stamp]
        
        # Scaler count:
        
        set count       [get32 $b 6 $o]
        
        # Format he heading:
        
        $w add "Scaler event $fstamp  End/Start/duration in seconds: $end/$start/$duration"
        
        #  Format the scalers:
        
        set offset 8
        $w add     "Scaler       Increment        Rate"
        for {set i 0} {$i < $count} {incr i} {
            set scaler [get32 $b $offset $o]
            incr offset 2
            $w add "[format %03d $i]          [format %6.0f $scaler]           [format %8.2f [expr {(1.0*$scaler)/$duration}]]"
        }
        
    }
}


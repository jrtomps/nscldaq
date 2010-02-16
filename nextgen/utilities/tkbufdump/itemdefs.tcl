#
#   Definitions of the ring event format.
#
#

#  Known item types:

# State changes:

set BEGIN_RUN   1
set END_RUN     2
set PAUSE_RUN   3
set RESUME_RUN  4

set ControlItems [list $BEGIN_RUN $END_RUN $PAUSE_RUN $RESUME_RUN]

# Documentation buffers (string lists)

set PACKET_TYPE         10
set MONITORED_VARIABLES 11

set StringListItems [list $PACKET_TYPE $MONITORED_VARIABLES]


# other types:

set PHYSICS_EVENT       30
set PHYSICS_EVENT_COUNT 31

# Sizes of stuff.

set TITLE_MAXSIZE 80

#  Procs that extract data from a list in a specific order:
#

# Give me a uint32_t
# Parameters:
#   list   - data
#   starts - Start offset
#   endian - 0 little endian, 1 bigendian
# Returns
#  a number.
proc get32 {list starts endian} {
    #  Get the two 16 bit words in question:
    
    set w1 [lindex $list $starts]
    set w2 [lindex $list [incr starts]]
    
    # Glue together depending on endian-ness:
    
    if {$endian} {
        #big:
        set hi $w1
        set lo $w2
    } else {
        #little:
        set lo $w1
        set hi $w2
    }
    return [expr {($hi << 16) | $lo}]
    
}
#
#  Give me a set of bytes
# Parameters:
#  list    - data
#  starts  - Start offset.
#  num     - Number of bytes.
# Returns:
#   list of bytes in appropriate order.
#
proc getbytes {list starts num} {
    #  Figure out the even number of bytes:
    #  and if there's one left over:
    
    set fullWords [expr {$num/2}]
    set halfWord  [expr {$num % 2}]
    
    set result [list]
    set c $starts
    for {set i 0} {$i < $fullWords} {incr i} {
        set word [lindex $list $c]
        incr c
        lappend result [expr {$word & 0xff}]
        lappend result [expr {$word >> 8}]
    }
    #  If there's an odd byte we need to append it as well:
    
    if {$halfWord} {
        set word [lindex $list $c]
        lappend result [expr {$word & 0xff}]
    }
    return $result
}
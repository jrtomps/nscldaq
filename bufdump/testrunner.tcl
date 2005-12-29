# Test driver.

package require tcltest


tcltest::configure -testdir [file dirname [info script]]
tcltest::configure -file    "*.test"
tcltest::configure -notfile junk
tcltest::configure -singleproc 1

# Custom match for lists that have numerical elements.

proc listMatch {expected is} {
    if {[llength $expected] != [llength $is]} {
        return 0
    }
    foreach e $expected i $is {
        if {$e != $i} {
            return 0
        }
    }
    return 1
}
tcltest::customMatch list listMatch


#    - Utilities used by multiple test files.

# makeCountingPattern low high
#       Make a counting pattern.
# low  - Low limit of pattern.
# high - Hich limit of pattern.
proc makeCountingPattern {low high} {
    for {set i $low} {$i <= $high} {incr i} {
        lappend result $i
    }
    return $result
}

# writeBinaryFile contents name
#      Create a binary file
# contents  - contents of the file  (produce with e.g binary format
# file      - name of the file.
#
proc writeBinaryFile {contents name} {
    set fd [open $name w]
    fconfigure $fd -encoding binary
    puts -nonewline $fd $contents
    close $fd
}


# Start and stop the dummy online data sources.
#

set sourcepid 0
proc startDummySource {} {
    global sourcepid
    set sourcepid [exec [file join [pwd] countingsource] 16 &]
}
proc stopDummySource {} {
    global sourcepid
    exec kill -9 $sourcepid
}

#  Return the 'canonical buffer header'
proc makeBufferHeader {} {

    #              sz tp   run  seq ent      lvl ssig     longsig
    lappend buffer 16 1  0 1234 1 0 0   0 0 0 3 0x0102 0x0102 0x0304 0 0
    return $buffer

}
#
#   Create scaler buffers
#
proc createScalerBuffer {{num 10}} {
    set size 27
    set header [makeBufferHeader]
    set header [lreplace $header 1 1 2]
    set header [lreplace $header 6 6 $num]
    set body   [list 1234 0  0 0 0  1230 0  0 0 0]
    for {set i 0} {$i < $num} {incr i} {
        lappend body $i 0
    }
    set buffer [concat $header $body]
    set buffer [lreplace $buffer 0 0 [llength $buffer]]
}

# Creates an event buffer
# where contents is a list of event bodies.
#  this function prepends lengths etc.
#
proc createEventBuffer {contents} {
    set buffer [makeBufferHeader]
    set buffer [lreplace $buffer 6 6 [llength $contents]]
    foreach event $contents {
        set buffer [concat $buffer [expr {[llength $event]+1}] $event]
    }
    set buffer [lreplace $buffer 0 0 [llength $buffer]]
    return $buffer
}
#   Creates a string for an NSCL DAQ buffer.
#   strings are null terminated... and terminated by, if necessary
#   two nulls to make them even length.
#
proc makeBufferString text {
    #  Null terminate ..and if necessary pad -> even.
    append text "\000"
    if {([string length $text] % 2) == 1} {
        append text "\000"
    }
    # Encode each pair of characters into a list element.
    # Note that our inistence on null termination above
    # means that their can be no zero length output list.
    # and that all strings are even lenght.
    #
    for {set i 0} {$i < [string length $text]} {incr i 2} {
        set twochars [string range $text $i [expr {$i+1}]]
        binary scan $twochars s1 ascii
        lappend result $ascii
    }
    return $result


}
# Make one of the string contents.
#    type     - the buffer type.
#    contents - A list of strings to put into the buffer.
#
proc createStringBuffer {type contents} {
    set buffer [makeBufferHeader]
    set buffer [lreplace $buffer 1 1 $type]
    set buffer [lreplace $buffer 6 6 [llength $contents]]
    foreach string $contents {
        set buffer [concat $buffer [makeBufferString $string]]
    }
    set buffer [lreplace $buffer 0 0 [llength $buffer]]
    return $buffer

}

tcltest::runAllTests


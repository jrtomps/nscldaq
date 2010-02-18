#
#  Formats physics events.
#  This is the most complex of the various formatters as it includes
#  a plugin architecture that allows users to extend it to produce
#  formatted dumps of the structure of packets within the event.
#

package require snit
set here [file dirname [info script]]
source [file join $here itemdefs.tcl]

#
# OPTIONS:
#   -body     - Body of the event.
#   -main     - Main widget of the tkbufdump program.
#   -order    - byte order indicator.
#   -type     - Type of the item being formatted.
#
# PUBLIC METHODS:
#  See comments in the public methods for most of the interface definition.
#
snit::type PhysicsFormatter {
    option -body
    option -main
    option -order
    option -type

    typevariable plugins
    typevariable packets
    typevariable packetFileName "packets.def";     # Standard packet filename.

    #--------------------------------------------------------------------------
    #
    #  Type level methods (and type-constructor)
    #
    
    #
    #  The type constructor:
    #  - Reads the packet definition file and loads it into the
    #    packets array.
    #  - Registers the default formatter for each defined packet.
    #
    typeconstructor {
        set packetDefinitionFile [file join $::here $packetFileName]
        PhysicsFormatter readPacketDefinitionFile $packetDefinitionFile
    }

        #
        #  Register a packet formatter with the formatter.  Packet formatters
        #  are called with a packet body and are expected to return text that
        #  displays the guts of that packet in the manner the author intends
        #  that packet to be displayed in the main window widget.
        #  Parameters:
        #   id        - Id of the packet for which the formatter will be
        #               registered.
        #   formatter - Command that will be invoked to format the packet.  The
        #               body of the packet as a list of word values will be appended
        #               to this command (which is usually a proc/method) the method
        #               is expected to return the formatted packet text...which
        #               can be several lines.
        # NOTE:
        #   Normally packet formatters for non-standard packet types are used
        #   in conjuntion with custom packet definition files.
        typemethod registerPacketFormater {id fmtcommand} {
            set normalizedId [expr $id]
            set plugins($index) $fmtcommand
        }
        #
        #  Read and process a packet definition file.
        #  Packet definition files are used to define new packet types to the
        #  base formatter.
        #  A packet definition file defines one packet per line. Each
        #  line contains in order (whitespace separated):
        #    short_name id long_name  version
        #
        #  Where:
        #    short_name is an abbreviation of the human readable packet name.
        #    id         is the packet id in any numeric format.
        #    long_name  is the long name of the packet.  If this contains
        #               whitespace it should either be quoted with ""'s or
        #               encapsulated in {}'s.
        #    version    is the version of the packet.
        # Parameters:
        #    packetDefinitionFile - Full path to the packet definition file
        # NOTE:
        #    The existing packet definitions are not proteted from overwrite.
        #
        typemethod readPacketDefinitionFile packetDefinitionFile {
            if {[file readable $packetDefinitionFile]} {
                set f [open $packetDefinitionFile r]
                while {![eof $f]} {
                    gets $f line
                    if {$line ne ""} {
                        set shortName [lindex $line 0]
                        set index     [expr [lindex $line 1]];  # Decimalizes.
                        set longName  [lindex $line 2]
                        set version   [lindex $line 3]
                        set packets($index) [list $shortName $longName $version]
                        set plugins($index) defaultPacketFormatter
                    }
                    
                }
                close $f
            } else {
                error "$packetDefinitionFile is not readable"
            }
        }
    #--------------------------------------------------------------------------
    #   Public methods.
    #
    
    # Format the data
    #  -body is assumed to contain a series of packets known to us.
    #        if we run into an unrecognized packet id, we will
    #        dump the rest of the event wordwise with a note that the
    #        packet is not recognized.
    #  Packets have the following format:
    #    +--------------------------+
    #    | uint32_t  words in packet|  (self inclusive)
    #    +--------------------------+
    #    | uint16_t Packet id       |
    #    +--------------------------+
    #    | packet body              |
    #        ...
    #    +--------------------------+
    # There is no padding between packets.
    #
    method format {} {
        set b $options(-body)
        set w $options(-main)
        set o $options(-order)
        set t $options(-type)
        
        set residual [llength $b]
        
        # Iterate over the event until it's exhausted.
        #
        while {$residual} {
            #
            # Get the packet header:
            #
            set size [get32 $b 0 $o]
            set type [lindex $b 2]
            
            #  Two cases the packet type is known in which case it has a formatter
            #  else not in which case the entire body is just dumped:
            
            if {[array names packets $type] ne ""} {
                set descrip $packets($type)
                set plugin  $plugins($type)
                set body [lrange $b 3 [expr {$size-1}]]
                set text [$self formatPacket $descrip $plugin $body]
                set b [lrange $b $size end]
                
            } else {
                $w add "Unknown packet type [format %04x $type] remainder of event:"
                set text [$self dumpRemainder $b]
                set b [list]
            }
            $w addlines $text
            set residual [llength $b]
        }
    }
    
    #  Default packet formatter
    # Parameters:
    #    Body of the packet.
    # Returns:
    #    the body formatted as a bunch of hex words, 8 per line.
    #
    proc defaultPacketFormatter body {
        set pos 1
        set result [list]
        foreach word $body {
            append result [format "%04x " $word]
            if {($pos % 8) == 0} {
                append result "\n"
            }
            incr pos
        }
        return $result
        
    }
    
    #------------------------------------------------------------------
    #   Private methods
    #
    
    #
    #  Produce the formatted dump of a packet.
    #
    # Parameters:
    #   description   - list that descripts the packet (see DESCRIPTION LIST below)
    #   plugin        - procedure to call to format the body of the packet.
    #   body          - body of the packet.
    # DESCRIPTION LIST:
    #    index        contents
    #     0           Short name of the packet.
    #     1           Long name of the packet.
    #     2           Version of the packet.
    # 
    #
    # Returns:
    #   text to insert in the format widget.
    #
    method formatPacket {description plugin body} {
        set short   [lindex $description 0]
        set long    [lindex $description 1]
        set version [lindex $description 2]
        
        set result "$short Packet: $long Version $version\n"
        append result [$plugin $body]
        
        return $result
        
    }
    #
    #  Dump the contents of body as a bunch of uninterpreted hex word.
    #  we dump 8 words per line.
    # Parameters:
    #   body    - Data to dump
    # Returns:
    #   text to insert into the dumper.
    #
    method dumpRemainder body {
        defaultPacketFormatter $body
    }
}


#
#  Register a packet formatter plugin with Physics Foramtter.
# Parameters:
#   id       - Packet id of the formatter.
#   fmtcmd   - Command that formats the packet.
#
proc registerPacketFormatter {id fmtcmd} {
    PhysicsFormatter registerPacketFormatter $id $fmtcommand;  # Delegate to class.    
}

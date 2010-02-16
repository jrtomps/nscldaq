package require snit

#
#  Formatter for unknown data types.
#
#  - If no plugin has been registered for this item type
#    just hex dumps the body word wise, 8 words/line.
#
# OPTIONS:
#  OPTIONS:
#    -body  - provides the event body as a list of numbers.
#    -main   - provides the main widget
#    -order  - 0 for little endian order, 1 for bit.
#    -type   - Type of the item.
#
#  METHODS:
#     format            - Formats the buffer into the main widget.
#     addUserFormatter  - registers a user formatter for an item type.
#
snit::type UnknownFormatter {
    option -body
    option -main
    option -order
    option -type
    
    #
    #  Array indexed by item type.
    #  The contents of each element is a user formatter for that item type.
    #  The user formatter is assumed to have the same interface as us.
    #
    typevariable userFormatters -array {}
    
    constructor args {
        $self configurelist $args
    }
    #
    #  Add a new user formatter
    # Parameters:
    #    type      - User defined type code (integer)
    #    formatter - Name of snit::type that can format this item type.
    #
    typemethod addUserFormatter {itemType formatter} {
        set userFormatters($itemType) $formatter
    }
    
    #
    #  Format an item.
    #  Parameters:
    #    None
    #  Implicit inputs
    #    all the options.
    #
    method format {} {
        set body    $options(-body)
        set type    $options(-type)
        set w       $options(-main)
        set order   $options(-order)
        set itemType $options(-type)
        
        $w add "Item type: $type "
        #
        #  See if we can delegate to a user formatter
        #
        if {[array names userFormatters $itemType] ne ""} {
            set formatter $userFormatters($itemType)
            $formatter userFormatter -body $body -type $type -main $w -order $order -type $itemType
            userFormatter format
            userFormatter destroy
        } else {
            set size [llength $body]
            set line ""
            for {set i 0} {$i < $size} {incr i} {
                append line [format "%04x " [lindex $body $i]]
                if {($i % 8 == 0) && ($i > 0)} {
                    $w add $line
                    set line ""
                }
            }
            $w add $line
        }
    }
}



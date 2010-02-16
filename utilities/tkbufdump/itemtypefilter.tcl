#  This type is a filter on item types.
#  filters are objects that must contain the applyFilter method that operates
#  on the type and body of the entire event, returning true if the filter
#  conditions are made.
#

package require snit
set here [file dirname [info script]]
source [file join $here itemdefs.tcl]

#
# ItemTypeFilter
#
# OPTIONS
#     -itemtypes         - List of item types that are acceptable
# METHODS
#     appliFilter        - Returns true if the item matches the filter condition.
#
snit::type ItemTypeFilter {
    option -itemtypes [list]
    
    constructor args {
        $self configurelist $args
    }
    
    #
    #  Apply the current filter.  The filter is true if the item type is
    #  in the set of types in options(-itemtypes).
    #
    # Parameters:
    #    itemType   - Type of the item.
    #    body       - Item body.
    #
    method applyFilter {itemType body} {
        return ([lsearch -exact $options(-itemtypes) $itemType] != 0)
    }
}


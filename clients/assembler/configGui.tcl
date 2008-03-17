#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


set configVersion 1.0
package provide configGui $configVersion
package require Tk
package require evbFileMenu
package require evbHelpMenu
package require Tktable
package require NodeWizard  


set Configuration [list];		# The configuration variable.
set HelpDirectory .;


#--------------------------------------------------------------------
#
#  Returns the row number of a node that matches the given node.
#  used to determine if a node is a duplicate of an existing one.
#
# Parameters:
#   node   - Name of the node.
# Returns:
#   Row number of 'node' else "" if 'node' is not in the table
# IMPLICIT INPUTS:
#   ::configArray - Table Contents.
#
proc dupeNode node {
    set row 1
    while {[array name ::configArray $row,0] ne ""} {
	set oldNode $::configArray($row,0)
	if {$oldNode eq $node} {
	    return $row
	}
	incr row
    }

    return ""
}
#--------------------------------------------------------------------
#
# Returns the row containing the specified ID or
# "" if there is no row with that id:
#
# Parameters
#   id    - Node id to look for.
# Returns:
#   Row number - if found.
#   ""         - if not found.
# IMPLICIT INPUTS:
#   ::configArray - Table contents.
#
proc dupeId id {
    set row 1
    while {[array name ::configArray $row,0] ne "" } {
	set oldId $::configArray($row,1)
	if {$oldId == $id} {
	    return $row
	}
	incr row
    }
    return ""

}
#------------------------------------------------------------------
#
#   Returns the row number of the existing trigger if the
#   node is different than the triggern node.
# Parameters:
#   node       - new proposed  node.
#   istrigger  - true if the proposed node is a trigger.
# Returns:
#   Row of existing trigger if node differs and istrigger is set.
#   empty string if not.
# IMPLICIT INPUTS:
#   ::configArray - Table contents.
#
proc dupeTrigger {node istrigger} {
    if {!$istrigger} {
	return ""
    }
    set row 1
    while {[array name ::configArray $row,0] ne ""} {
	set oldNode      $::configArray($row,0)
	if {$::configArray($row,4) eq "Trigger"} {
	    if {$oldNode eq $node} {
		return ""
	    } else {
		return $row
	    }
	}
	
	incr row
    }
}


#-----------------------------------------------------------------
#
#   The configuration used by the file menu commands is a list
#   structured entity as that's easiest to save/restor.
#   The table, however uses an array.  This rewrites the list
#   from the table array.
# Parameters:
#    table   - The table widget.
#
# IMPLICIT INPUTS:
#   ::configArray - the array that describes the table contents.
# IMPLICIT OUTPUTS:
#   ::Configuration - The configuration list.
proc updateConfigurationList {table} {
    set ::Configuration [list]

    set rows [$table cget -rows]
    for {set r 1} {$r < $rows} {incr r} {
	set item [list]
	lappend item $::configArray($r,0) $::configArray($r,1) $::configArray($r,2) $::configArray($r,3)
	if {$::configArray($r,4) eq "Trigger"} {
	    lappend item 1 0 0
	} else {

	    lappend item 0 [lindex $::configArray($r,4) 0] [lindex $::configArray($r,4) 2]
	}
	lappend ::Configuration $item
    }
}


#----------------------------------------------------------------
#
#  Produces the matchInfo string:
# Parameters:
#   trigger  - true if this is a trigger.#
#   width    - Width if not.
#   offset   - offset if not.
#
proc matchInfo {trigger width offset} {
    if {$trigger} {
	return "Trigger"
    } else {
	return "$width / $offset"
    }
}


#---------------------------------------------------------------
#  If a required field is missing, returns the name of the 
#  first missing field found.
#
# Parameters:
#   node  - Node name.
#   id    - The id of the node to check.
#   path  - The path to the node's readout program.
#
proc missingNodeElement {node id path} {
    if {$node eq ""} {return "Node"}
    if {$id   eq ""} {return "Id"}
    if {$path eq ""} {return "Readout Path"}
    
    return ""
}


#----------------------------------------------------------------
#
#  Creates a new node and inserts it in the table.
#  This can either add a row to a table or replace an existing row.
#  Existing rows are replaced if:
#    - The node entered is specified to be a trigger, there's an 
#      existing trigger node and the user confirms the overwrite.
#    - The node name duplicates an existing node the user confirms
#      the overwrite.
#    - The node id duplicates an exiting node and the user confirms
#      the ovewrite.
#
#  
# Parameters:
#   table   - The table in which to insert the row.
#  
proc newNode table {
    
    # Prompt for the node using the enter ndoe wizard.
    # If the return value is not Finished, the user chose not to 
    # complete the wizard.
    #

    set wizard [newNodeWizard %AUTO%]
    set result [$wizard start]

    if {$result eq "Finished"} {
	set node      [$wizard cget -node]
	set id        [$wizard cget -id]
	set path      [$wizard cget -path]
	set args      [$wizard cget -args]
	if {$args eq ""} {set args " "}
	set istrigger [$wizard cget -istrigger]
	set width     [$wizard cget -matchwindow]
	set offset    [$wizard cget -matchoffset]

	set missing [missingNodeElement $node $id $path]
	if {$missing ne ""} {
	    tk_messagebox -type ok -icon error -title "Missing required field" \
		-message "You did not provide a value for the required field $missing"
	    return
	}

	set noderow [dupeNode $node]
	set idrow   [dupeId   $id]
	set trigrow [dupeTrigger $node  $istrigger]



	# If we have both a duplicate node and a duplicate id,
	# they must be the same row, else replacement is not possible.

	if {($noderow ne "") && ($idrow ne "") && ($noderow != $idrow)} {
	    tk_messageBox -type ok -icon error -title {Invalid duplication} \
		-message {Both the Node name and Id duplicate existing ones, but not in the same row}
	    return 
	}


	# If we have a duplicate trigger and a duplicate node, or duplicate id they
	# must be the same row, or again, replacement is not possible.
	#
	if {($trigrow ne "") && ($noderow ne "") && ($trigrow != $noderow)} {
	    tk_messageBox -type ok -icon error -title {Invalid duplication} \
		-message {You've specified a new trigger node, and a duplicate node that are not on the same row}
	    return
	}
	if {($trigrow ne "") && ($idrow ne "") && ($trigrow != $idrow)} {
	    tk_messageBox -type ok -icon error -title {Invalid Duplication} \
		-message {You've specified a new trigger node, and a duplicate id that are not on the same row}
	    return
	}

	# At this point, any duplications are legal... that is they all reference the
	# same row.

	set duplications 0

	set dupMessage "You've specified duplications for: "
	if {$noderow ne ""} {
	    append dupMessage " the node name"
	    incr duplications
	    set duprow $noderow
	}
	if {$idrow ne ""} {
	    append dupMessage " the node id"
	    incr duplications
	    set duprow $idrow
	}
	if {$trigrow ne ""} {
	    append dupMessage " the trigger"
	    incr duplications
	    set duprow $trigrow
	}
	
	append dupMessage ". If you accept this duplication, an existing row will be overwritten."
	append dupMessage " continue?"
	if {$duplications > 0} {
	    set answer [tk_messageBox -icon question -type yesno -title {Ovewrite row?} \
			    -message $dupMessage]
	    if {$answer eq "yes"} {
		$table set row $duprow,0 [list $node $id $path $args \
					      [matchInfo $istrigger $width $offset]]
	    }
	} else {
	    $table insert rows end
	    set row [expr [$table cget -rows] - 1]
	    $table set row $row,0 [list $node $id $path $args \
				      [matchInfo $istrigger $width $offset]]
	}
	updateConfigurationList $table
	
    } else {
	$wizard destroy
	return 
    }
}

#----------------------------------------------------------------
#
#  Create the configuration user interface
#
# Parameters:
#   top    - this is the top level widget in which the 
#            GUI will be built (may be . or another toplevel).
#





proc createUI top {
    global HelpDirectory
    global configVersion

    #
    # The prefix for other widgets is "" if top is .
    #
    if {$top eq "."} {
	set prefix ""
    } else {
	set prefix $top
    }

    # Create the menu bar and menus:

    set menubar [menu $top.menubar -tearoff false]
    $top config -menu $menubar

    menu $menubar.file -tearoff false
    menu $menubar.help -tearoff false

    $menubar add cascade -label File -menu $menubar.file
    $menubar add cascade -label Help -menu $menubar.help


    fileMenu %AUTO%   -menu $menubar.file -configvar Configuration
    helpMenu %AUTO%   -menu $menubar.help -helpdir  $HelpDirectory \
	-abouttext "Event builder configuration builder version $configVersion"

    set table [table $prefix.table  -cols 5 -titlerows 1 \
		   -variable ::configArray -rows 1 -colwidth 20 -ellipsis ...] 
    $table set row 0,0 [list Node Id [list Readout Program] [list Readout args] Window/Offset]

    # the new button...

    set new [button $prefix.new -text {New...} -command [list newNode $table]]


    # Layout the table:

    grid $table -sticky nsew
    grid $new   -sticky w

    # save the gui elements in the configGui array

    set ::configGui(table) $table
    set ::configGui(new)   $new

}
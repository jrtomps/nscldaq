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
package require dialog
package require EnterNode
package require EnterReadout
package require NodeTimingEntry

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
#   OnNew - This is called when the user accepts the new
#           command.  The table widget contolling this
#           configuration is set back to a single row (the titles).
#
# Parametrs:
#   table   - The name of the table widget.
#
proc onNew table {


    # Unfortunately there does not seem to be a way to kill off from
    # 1 -> end... at least not as simply as the code below.

    while {[$table cget -rows] > 1} {
	$table delete rows 1 1
    }

    
}
#----------------------------------------------------------------
#
#  A new file has been read in.  We must:
#  - Clear our self
#  - Convert the configuration listing into
#    the table format.
# Parameters:
#    table   - The configuration table.
#
#
proc onOpen {table} {
    onNew $table

    set row 1
    foreach item $::Configuration {
	set node    [lindex $item 0]
	set id      [lindex $item 1]
	set readout [lindex $item 2]
	set rdoargs [lindex $item 3]
	set istrig  [lindex $item 4]
	set window  [lindex $item 5]
	set offset  [lindex $item 6]
	
	$table insert rows end
	$table set row $row,0 [list $node $id $readout $rdoargs \
				  [matchInfo $istrig $window $offset]]

	incr row
    }
}
#----------------------------------------------------------------
#
#   User has requested to edit the node on a specific row.
#   we've been dispatched to by a right click.
#
# Parameters:
#   table  - The table widget on which the right click was done.
#   row    - The row in which the right click was done.
# Implicit Inputs/Outputs
#   configArray - The backing store array for the table.
#
#
proc newNodeId {table row} {
    global configArray

    # Create a dialog whose work area is an EnterNode widget
    # to prompt for the new node info:

    dialog .newnode -buttons [list Ok Cancel] -title {New Node Information}
    set    workarea   [.newnode workArea]

    EnterNode $workarea.nodeinfo -node $configArray($row,0)   \
	-id $configArray($row,1)
    .newnode configure -workarea $workarea.nodeinfo
    
    set result [.newnode execute]


    #   If the user clicked the ok button. We can accept the results
    #   of the dialog.  It is an error to have duplicated a node or id.
    #   as a result of the edit.

    if {$result eq "Ok"} {
	set node [$workarea.nodeinfo cget -node]
	set id   [$workarea.nodeinfo cget -id]
	set path $configArray($row,2)
	set args $configArray($row,3)
	set triginfo $configArray($row,4)
	if {$triginfo eq "Trigger"} {
	    set istrigger 1
	    set window    0
	    set offset    0
	} else {
	    set istrigger 0
	    set triglist [split $triginfo /]
	    set window   [lindex $triglist 0]
	    set offset   [lindex $triglist 1]
	}
	set noderow [dupeNode $node]
	set idrow   [dupeId   $id]

	# If there's another node or id by that name,
	# reject in error... otherwise update the row

	if {($noderow ne "") && ($noderow != $row)} {
	    tk_dialog .dupname \
		{Duplicate node} \
		"$node is a node naame that's already in the configuration" \
		error 0 Dismiss
	} elseif {($idrow ne "") && ($idrow != $row)} {
	    tk_dialog .dupid      \
		{Duplicate id}    \
		"$id is a node id that's already in the configuration" \
		error 0 Dismiss
	} else {
	    set configArray($row,0) $node
	    set configArray($row,1) $id
	}
	
    }
    destroy $workarea.nodinfo
    destroy .newnode

}

#----------------------------------------------------------------
#
#  Processes right clicks in the table on the program path
#  or the program arguments columns.  This action prompts
#  for a replacement to the current values of those two columns.
#  We do this by bringing up a dialog containing an EnterReadout
#  widget in its work area.
#
# Parameters:
#   table     - The table in which the right click happened.
#   row       - The row of the table that was hit.
#
# Implicit Inputs/Outputs:
#   configArray  - The array that contains the table backing store.
#
proc newProgramArgs {widget row} {
    global configArray

    # Create the dialog and insert the EnterReadout node inside it.

    dialog .newpath -buttons [list Ok Cancel] -title {Path/args}
    set workarea [.newpath workArea]

    EnterReadout $workarea.readout -path $configArray($row,2) \
	-args $configArray($row,3)
    .newpath configure -workarea $workarea.readout

    set result [.newpath execute]

    #  this requires no real validation. If the user clicked
    #  Ok, then we can load up the configuration array with the
    #  new values of the path and args:

    if {$result eq "Ok"} {
	set configArray($row,2) [$workarea.readout cget -path]
	set configArray($row,3) [$workarea.readout cget -args]
    }

    destroy $workarea.readout
    destroy .newpath
}

#----------------------------------------------------------------
#
#  Process right clicks in a table cell that has matching information.
#  The cell contents will either be "Trigger" or a / separated list
#  of the window width and offset.  These will be used
#  to set up a dialog whose work area is a nodeTimingEntry widget.
#  If the user accepts this, and no other node is currently the trigger,
#  we will accept the edit.
#
# Parameters:
#   table       - Widget in which the right click was done.
#   row         - Row in which the click was done.. implicitly this is
#                 column 4.
# Implicit Inputs/Outputs:
#   configArray - The configuration array that is the table backing store.
#
proc newMatching {table row} {
    global configArray
    
    # Figure out the current values:

    set node $configArray($row,0)
    set timingInfo $configArray($row,4)
    if {$timingInfo eq "Trigger"} {
	set isTrigger   1
	set width       1
	set offset      0
    } else {
	set isTrigger   0
	set width       [lindex [split $timingInfo /] 0]
	set offset      [lindex [split $timingInfo /] 1]
    }

    # Create the dialog and insert a nodeTimingEntry 
    # item in the work area, the nodeTimingEntry will be configured
    # as per the current table values, which have been pulled out into
    #  isTrigger, width and offset.
    
    dialog .newmatch -buttons [list Ok Cancel] -title {Trigger Match Info}
    set workarea [.newmatch workArea]
    nodeTimingEntry $workarea.timing     \
	-istrigger $isTrigger            \
	-matchwindow $width              \
	-matchoffset $offset
    .newmatch configure -workarea $workarea.timing

    set result [.newmatch execute]

    if {$result eq "Ok"} {
	set isTrigger [$workarea.timing cget -istrigger]
	set width     [$workarea.timing cget -matchwindow]
	set offset    [$workarea.timing cget -matchoffset]

	set dupRow [dupeTrigger $node $isTrigger]
	if {($dupRow ne "") && ($dupRow != $row)} {
	    tk_dialog .duptrigger  \
		{Duplicate Trigger node} \
		"There is already a trigger node in the configuration and there can be only one" \
		error 0 Dismiss
	    
	} else {
	    set configArray($row,4) [matchInfo $isTrigger $width $offset]
	}

    }

    destroy $workarea.timing
    destroy .newmatch
}

#----------------------------------------------------------------
#
#  Process right clicks in the table.
#
# Parameters:
#   widget    - The table widget.
#   x         - The window x coordinate of the click.
#   y         - The window y coordinate of the click.
# Implicit inputs/outputs:
#   configArray - The backing store for the table.
#
proc onRightClick {widget x y} {
    global configArray

    # Convert to a row/column.
    
    set tableIndex [split [$widget index @$x,$y] ,]
    set row        [lindex $tableIndex 0]
    set column     [lindex $tableIndex 1]

    # If the index to the array does not exist,
    # just return.

    if {[array name ::configArray $row,$column] eq ""} {
	return
    }
    # If the row is 0, that's the title row, so return:

    if {$row == 0} {
	return
    }
    #  We now will dispatch to the appropriate processor depending on the
    # column.
    # here are the column definitions:

    set NODE    0
    set ID      1
    set PROGRAM 2
    set ARGS    3
    set MATCH   4

    switch -exact -- $column       \
	$NODE - $ID {
	    newNodeId      $widget $row
	}                       \
	$PROGRAM - ARGS {
	    newProgramArgs $widget $row
	}                       \
	$MATCH {
	    newMatching    $widget $row
	}                       \
	default {
	}

    # By this time the data may have changed, so update the
    # config list:

    updateConfigurationList $widget

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


    set fm [fileMenu %AUTO%   -menu $menubar.file -configvar ::Configuration ]

    helpMenu %AUTO%   -menu $menubar.help -helpdir  $HelpDirectory \
	-abouttext "Event builder configuration builder version $configVersion"

    set table [table $prefix.table  -cols 5 -titlerows 1 \
		   -variable ::configArray -rows 1 -colwidth 20 -ellipsis ...] 
    $table set row 0,0 [list Node Id [list Readout Program] [list Readout args] Window/Offset]

    $fm configure  -newcommand [list onNew $table] \
	-opencommand [list onOpen $table]

    # the new button...

    set new [button $prefix.new -text {New...} -command [list newNode $table]]


    # Layout the table:

    grid $table -sticky nsew
    grid $new   -sticky w

    # save the gui elements in the configGui array

    set ::configGui(table) $table
    set ::configGui(new)   $new

    # Bind the table right click to uh.. onRightClick so that individual cells can be
    # edited appropriately.

    bind $table <Button-3> [list onRightClick $table %x %y]

}
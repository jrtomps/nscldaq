package require Tk
package require BWidget
package require snit

set here [file dirname [info script]]
source [file join $here itemdefs.tcl]

#
#  The megawidget implemented below is a dialog that allows you to prompt
#  the user for a filter condition that is based on item types.
# OPTIONS:
#    -itemtypes       - The set of item types.
#    -okcommand       - Script supplied for the OK click.
#    -cancelcommand   - Script supplied for the Cancle click.
#    -helptext        - HTML of help text for the widget.
# METHODS:
#   modal             - Start application modal operation. Will not return until
#                       either the ok/cancel buttons are clicked or 
#   dismiss           - Used by modal action handlers to end modality.
#   nameToId          - Translate an item type name to an item id.
#
# LAYOUT:
#   THe dialog is divided into a top and bottom frame.  The top frame is
#   called the 'action area' and is where all the configuration controls are.
#   The bottom is called the 'command area' and contains the Ok, Cancel and
#   Help buttons:
#
#   +--------------------------------------------------------+
#   |    Item Types                       Acceptable Types   |
#   |  +------------------------+    +---------------------+ |
#   |  \ list of allowed items  / -> \ List of current     / |
#   |  /  for filter            \ <- / accepted item types \ |
#   |  +------------------------+   +----------------------+ |
#   | User type: [          ]  <Add>                         |
#   +--------------------------------------------------------+
#   |  <Ok>    <Cancel>  <Help>                              |
#   +--------------------------------------------------------+
#
#  The right most of the two listboxes above will scroll
# EVENT BINDINGS:
#   <Double-MB1> an item in the left box moves it to the right one
#   <Double-MB1> an item in the right box moves it to the left one unless
#   it's a custom type, in which case it's just removed.
#   <Enter> in the User type: entry validates and puts the value in the
#   acceptable type box.
# ENTRY Validation:
#    * Must be a number.
#    * Must be    0 < n < 65536
#

snit::widget TypeFilterDialog {
    hulltype toplevel
    
    option -itemtypes           -default [list]             \
                                -configuremethod setItems   \
                                -cgetmethod      getItems
    option -okcommand -default [list]
    option -cancelcommand -default [list]
    option -helptext -default {<html><head><title>No help</title><body><h1>No help has been provided</h1></body</html>}
    
    variable fakewindow ""
    
    #  Easy access widgets:
    
    variable choices;                # Left list box
    variable acceptable;             # Right list box.
    variable user;                   # User type entry.
    variable helpframe [list];       # nonempty - help is displayed.
    
    #  Set of known item types:
    
    variable knownItems 
        
        
    
    constructor args {
        
        # Create the action and command frames.
        
        set action  [frame $win.action    -relief ridge -borderwidth 3]
        set command [frame $win.command]
        
        # Now layout the action area.
        
        set choices     [listbox $action.choices]
        
        ArrowButton $action.add     -command [mymethod addChoiceToAcceptable]  \
                                    -dir right
        ArrowButton $action.remove  -command [mymethod removeChoiceFromAcceptable] \
                                    -dir left
        
        set acceptable  [listbox $action.acceptable -yscrollcommand [list $action.ascroll set]]
        scrollbar $action.ascroll -orient vertical -command [list $acceptable -yview]
        
        label $action.userlabel -text {User Item Type}
        set user [entry $action.user -validatecommand [mymethod validateUserItem] ]
        
        
        grid $choices           -row 0 -column 0 -rowspan 2
        grid $action.add        -row 0 -column 1
        grid $acceptable        -row 0 -column 2 -rowspan 2
        grid $action.ascroll    -row 0 -column 3 -rowspan 2 -sticky nsw
        grid $action.remove     -row 1 -column 1
        
        grid $action.userlabel  -row 2 -column 0
        grid $user              -row 2 -column 1
        
        # Layout the command area
        
        button $command.ok      -text Ok        -command [mymethod dispatch -ok]
        button $command.cancel  -text Cancel    -command [mymethod dispatch -cancel]
        button $command.help    -text Help      -command [mymethod displayHelp]
        
        grid $command.ok $command.cancel $command.help -sticky w
        
        # Layout the areas in the hull
        
        grid $action    -sticky nsew
        grid $command   -sticky wns
        
        # Stock the choices list box.
        
        array set knownItems   [list                \
        "Begin"         $::BEGIN_RUN                \
        "End"           $::END_RUN                  \
        "Pause"         $::PAUSE_RUN                \
        "Resume"        $::RESUME_RUN               \
        "Packets"       $::PACKET_TYPE              \
        "Variables"     $::MONITORED_VARIABLES      \
        "Event"         $::PHYSICS_EVENT            \
        "Triggers"      $::PHYSICS_EVENT_COUNT      \
    ]
        foreach item [lsort [array names knownItems]] {
            $choices insert end $item
        }
        
        # Establish event bindings.
        
        bind $choices       <Double-1> [mymethod addChoiceToAcceptable]
        bind $acceptable    <Double-1> [mymethod removeChoiceFromAcceptable]
        bind $user          <KP_Enter> [mymethod validateUserItem]
        bind $user          <Return>   [mymethod validateUserItem]
        
        
        # Last process the args as -itemtypes will rearrange what's in the list
        # boxes.
        #
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    #  Option processing
    #
    
    #
    #  Process -itemtypes configuration set the list boxes accordingly.
    #  you can currently only configure in known items.
    #
    method setItems {option value} {
        # All elements of value must be valid
        
        foreach item $value {
            if {[array names knownItems $item] eq ""} {
                error "Invalid value for -itemtypes option: $item"
            }
        }
        #  Items that are in the list go in the acceptable box, while those not
        # in the choices box
        
        $choices delete 0 end
        $acceptable delete 0 end
        
        foreach item [lsort [array names knownItems]] {
            if {[lsearch -exact $value $item] != -1} {
                $acceptable insert end $item
            } else {
                $choices insert end $item
            }
        }
        
    }
    #
    #  process -itemtypes cget, returns a possibly empty list of the
    #  item types in the acceptable listbox.
    #
    method getItems {option} {
        return [$acceptable get 0 end]
    }
    #---------------------------------------------------------------------------
    #
    # Public methods
    #
    
    # Turn the dialog into a modal one.  This is done by creating a hidden frame
    # and grabbing all the events until the frame is destroyed.   Use
    # dismiss to destroy the hidden frame (from inside an action routine).
    #
    method modal {} {
        if {$fakewindow eq ""} {
            set fakewindow [frame $win.fake]
            wm deiconify $win
            focus $win
            grab $win
            tkwait window $fakewindow
        }
    }
    #
    #  Dismiss the modality of the dialog by destroying $fakewindow:
    #
    method dismiss {} {
        destroy $fakewindow
        set fakewindow [list]
    }
    #  Translate an item type string to an item id.
    # Parameters:
    #   name  - Item type name string.
    # Returns:
    #   item id or error if there is no match.
    #
    method nameToId name {
        return $knownItems($name)
    }
    #-------------------------------------------------------------------------
    #
    #   Internal event handlers
    #

    #  Adds the currently selected item from choices to the acceptable
    #  list box.  The choice is also removed from the
    #  choices listbox so that it cannot be added again.
    #  The acceptable list box is sorted alphabetically once that's all done.
    #
    method addChoiceToAcceptable  {} {
        
        # Get/remove the choice from choices:

        set selection [$self getAndRemoveSelection $choices]
        if {$selection eq ""} {
            return
        }

        set current [$acceptable get 0 end]
        lappend current $selection
        
        $self setItems -itemtypes $current
        
    }
    # Remove the currently selected item from acceptable.  In appearance.
    # This item is deleted from the acceptable list box.. If it's
    # a known item it is also added to the choices box.
    #
    method removeChoiceFromAcceptable {} {
        set selection [$self getAndRemoveSelection $acceptable]
        if {$selection eq ""} {
            return
        }
        
        if {[array names knownItems $selection] ne ""} {
            set current [$choices get 0 end]
            lappend current $selection
            set current [lsort $current]
            
            $choices delete  0 end
            foreach choice $current {
                $choices insert end $choice
            }
        }
    }
    #------------------------------------------------------------------------
    #
    #  Private utility functions
    #
    
    # Get the active element of a list box remove it and return its value
    #
    # Parmeters:
    #    box  - Widget of box.
    #
    method getAndRemoveSelection {box} {
        set i           [$box index active]
        set selection   [$box get $i]
        $box delete $i
        return $selection
    }
}




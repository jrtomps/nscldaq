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



package provide dialog 1.0
package require Tk
package require snit


#
#   the dialog widget provides a generic snit dialog.  A dialog
#   is a toplevel widget that consists of two frames vertically
#   stacked.  The top frame, called the work area, has some arbitrary
#   set of widgets in which the user fills in things, selects stuff,
#   or maybe just reads stuff (e.g. tk_messageBox like).
#   The bottom frame, called the action area,  has a row of labelled buttons.
#   
#   In this widget, the client provides a widget to place in the work area.
#   The client also provides a list of button labels
#   for the action area.
#
#   Once the dialog is setup, the client invokes the execute method which 
#   displays the dialog, and makes it modal.  
#   
#   The execute button will return the text label of the button
#   that was clicked or "" if the dialog was just destroyed via e.g. the X banner decoration.
#
# Options:
#   -buttons     - List of button labels for the action area.
#   -title       - Title of top level widget for the dialog.  This defaults to whatever
#                  toplevel does.
#   -workarea    - The widget to put in the work area.
#
# Methods:
#   execute     - Lays out the widget, modalizes it and waits for the 
#                 user to complete interaction with it.
#

snit::widget dialog {
    hulltype  toplevel
    option -buttons
    option -title
    option -workarea

    variable hiddenWindow
    variable returnValue [list]

    # The constructor just withdraws the toplevel widget, and processes
    # the configuration args:
    # the execute method does all the heavy lifting
    #

    constructor args {
	wm withdraw $win
	$self configurelist $args

	# Containers for the user's stuff -- need to be made now so the client can
	# make the work area widget.
	
	frame $win.work
	frame $win.action




    }
    # The destructor ensures invokes onDestory with win to I'm trying
    # probably unsuccessfully to deal with the edge case that some
    # event handler of the widget in the work area destroys the
    # dialog.
    #
    destructor {
	$self onDestroy $win
    }

    # Get the work area so the user can put his stuff underneath it in the
    # widget hierarchy.  This works around an issue with invisible labels
    # found when testing if I workarea widgets like $win.mywidget.
    #
    
    method workArea {} {
	return $win.work
    }

    #  The execute widget does all the work.  
    #  - Build/layout the widget.
    #  - modalize 
    #  - tkwait on the destruction of $hiddenWindow.
    #

    method execute {} {
	# It's an error not to provide a -workarea:

	if {$options(-workarea) eq ""} {
	    error "Required option -workarea unconfigured"
	}

	wm deiconify $win

	# manage the work area so it fills the frame:

	grid $options(-workarea) -in $win.work  -sticky nsew

	# Create the buttons and manage them as well:

	set bindex 0
	foreach button $options(-buttons) {
	    button $win.action.b$bindex -text $button -command [mymethod onButton [list $button]]
	    grid $win.action.b$bindex -row 0 -column $bindex  -sticky nsew
	    incr bindex

	}

	# Manage the elements:

	grid $win.work   -sticky nsew
	grid $win.action -sticky nsew


	# Create the hidden window:

	set hiddenWindow [frame $win.hidden]

	# Modalize and wait  for the hidden window to be destroyed.

	bind $win <Destroy> [mymethod onDestroy %W]

	focus $win
	grab $win

	tkwait window $hiddenWindow
	grab release $win

	catch {bind $win <Destroy> ""};	# When win is destroyed we don't want our method called.

	# Destroy the buttons and forget the gridding.. Then we ought to be able to 
	# re-use this.  The catch  is in case we are being destroyed:


	catch {
	    grid forget $options(-workarea)
	    foreach child [winfo children $win.action] {
		destroy $child
	    }
	}
	if {[winfo exists $win]} {
	    return $returnValue
	} else {
	    return ""
	}
    }
    #    If a button is clicked, set the return value to its text (our argument)
    #   and destroy the hidden window to wake the app up:
    # 
    method onButton {text} {
	set returnValue $text
	destroy $hiddenWindow
    }
    #  If a widget in $win is destroyed, and if that widget is $win
    #  Set the return value  to "" and destroy the hidden window
    #  Note that very likely the hidden window is already destroyed, and
    #  since returnValue is initialized to empty this is a nice
    #  but never called bit of code.
    #

    method onDestroy {widget} {
	if {$widget eq $win}  {
	    set returnValue ""
	    destroy $hiddenWindow
	}
    }
    
}
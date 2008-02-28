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



package provide evbHelpMenu 1.0
package require Tk
package require snit
package require Iwidgets

#  This package implements the help menu for
#  the event builder user interface.
#  The help menu has two command entries:
#    Topics...   - Displays the help topics in the help directory.
#    About       - Displays some about text in a message info box.
#
snit::type helpMenu {
    option -menu
    option -helpdir .
    option -abouttext 

    constructor args {
	$self configurelist $args

	# Build the menu items:

	set menu $options(-menu)

	$menu add command -label Topics... -command [mymethod OnTopics]
	$menu add command -label About...  -command [mymethod OnAbout]

	
    }
    # Respond to the Topics.. click by starting up a help browser on the
    # index.html file in the directory -helpdir.
    #
    method OnTopics {} {
	set path [split $self :]
	set name [lindex $path end]
	set widget .$name

	if {![winfo exists $widget]} {
	    iwidgets::hyperhelp $widget -helpdir $options(-helpdir)
	}
	$widget  showtopic index

	wm deiconify $widget
	
    }
    # Respond to the About.. click by firing up a tk_messageBox with the
    # specified about text:

    method OnAbout {} {
	tk_messageBox -icon info -type ok -title "About:" -message $options(-abouttext)
    }
}

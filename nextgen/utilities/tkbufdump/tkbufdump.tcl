#! /usr/bin/tclsh

#  This program is a GUI event inspector.
#  It uses ringselector in sampling mode to accept events from an
#  event source.  The software free runs event acceptance and, when requested,
#  provides the next event from the stream.
#
package require Tk
package require snit
package require BWidget

set here [file dirname [info script]]

source [file join $here dataSourcePrompter.tcl]

#  mainwindow
#
#   User interface:
#  +-----------------------------------------------------------+
#  |   menubar                                                 |
#  +-----------------------------------------------------------+
#  |                                                           |
#  ~     Display area                                          ~
#  |                                                           |
#  +-----------------------------------------------------------+
#  | status bar                                         | [>]  |
#  +-----------------------------------------------------------+
#
# When making the menu, the assumption is that this widget lives
# pretty high in the top level widget hierarchy.. and as such
# can control the 'real menubar'.
#

snit::widget mainwindow {
    option -statustext {}
    option -maxlines   200
    #
    #   User event handlers:
    #
    
    
    constructor {args} {

	
	set top [winfo toplevel $win]
	
	# Create the menubar:

	set menubar [menu $top.menubar]
	$top config -menu $menubar
	
	# top level menus in the bar:

	foreach m [list File Edit Filter] {
	    set $m [menu $menubar.m$m]
	    $menubar add cascade -label $m -menu $menubar.m$m
	}
	menu $menubar.help
	$menubar add cascade -label Help -menu $menubar.help

	$File add command -label Open -command [mymethod openSource]
	$File add separator 
	$File add command -label Exit -command exit

	#  The top of the frame is a big fat text widget
	# 25 lines by 80 characters wide.
	#
	
	text      $win.dump      -width 80 -height 25 -relief ridge \
	                         -yscrollcommand [list $win.yscroll set] \
				 -state disabled
	
	scrollbar $win.yscroll   -command [list $win.dump yview] -orient vertical

	grid $win.dump  $win.yscroll -sticky nsew

	frame $win.bottom -relief ridge
	label $win.bottom.statusbar
	ArrowButton $win.bottom.next -dir right
	
	
	pack $win.bottom.statusbar -side left
	pack $win.bottom.next     -side right
	grid $win.bottom -sticky nsew
	

    }
    #-------------------------------------------------------------------------
    #  Public methods
    #
    
    
    #
    #  Clear the text:
    #
    method clear {} {
	$win.dump delete 1.0 end
    }
    #
    #  Insert a line of text.
    #
    method add line {
	$win.dump configure -state normal
	$win.dump insert end "$line\n"
	
	set lines [$win.dump count -lines 1.0 end]
	if {$lines > $options(-maxlines)} {
	    $win.dump delete 1.0 2.0
	}
	$win.dump see end
	$win.dump configure -state disabled
    }
    #
    #   Adds multiple lines of text:
    #
    method addlines lines {
	$win.dump configure -state normal
	$win.dump insert end "$lines\n"
	set lines [$win.dump count -lines 1.0 end]
	if {$lines > $options(-maxlines)} {
	    set deleteCount [expr {$lines - $options(-maxlines)}]
	    $win.dump delete 1.0 $deleteCount.0
	}
	$win.dump see end
	$win.dump configure -state disabled
    }
    #--------------------------------------------------------------------------
    #
    #  Action handlers:
    #
    
    #  openSource - This is called when the File->Open button is clicked.
    #               The user is prompted for the host and user name from which
    #               data will be taken (Modal)  The ringselector program
    #               will be started on a pipe with an fd handler established
    #               to accept data.
    #
    method openSource {} {
	
    }
}


mainwindow .w
pack .w








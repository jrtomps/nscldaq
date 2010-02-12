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

source [file join $here datasource.tcl]

set daqbin /usr/opt/daq/10.0/bin

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
    
    # Some saved widgets.
    
    variable statusbar
    variable text
    
    # File descriptor open on pipe to event source.
    
    variable fd {}

    # This variable is set to 1 when the user requests an event from
    # the data source.  It will be reset when the even has
    # been selected and displayed (selection implies matching any filter
    # criteria that are in place).
    #
    variable sample 0
    
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
	
	set text [
	    text      $win.dump      -width 80 -height 25 -relief ridge \
				     -yscrollcommand [list $win.yscroll set] \
				     -state disabled]
	
	scrollbar $win.yscroll   -command [list $win.dump yview] -orient vertical

	grid $win.dump  $win.yscroll -sticky nsew

	frame $win.bottom -relief ridge
	set statusbar [label $win.bottom.statusbar]
	ArrowButton $win.bottom.next -dir right -command [mymethod requestSample]
	
	
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
	$text configure -state normal
	$text insert end "$line\n"
	
	set lines [lindex [split [$text index end] .] 0]
	if {$lines > $options(-maxlines)} {
	    $text delete 1.0 2.0
	}
	$text see end
	$text configure -state disabled
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
	dataSourcePrompt .p -okcommand [list .p dismiss] -cancelcommand [list destroy .p]
	.p modal
	#
	#  If .p does not exist it was cancelled, otherwise we can fetch the
	#  host and the username from the widget:
	#
	
	if {[winfo exists .p]} {
	    if {$fd ne ""} {
		close $fd
	    }
	    set host [.p cget -host]
	    set user [.p cget -user]
	    
	    $statusbar configure -text "Data from $host:$user"
	    
	    destroy .p
	    
	    set fd [open "|$::daqbin/ringselector --formatted --sample=PHYSICS_EVENT" r]
	    fconfigure $fd -buffering line
	    fileevent $fd readable [mymethod event $fd]
	}
    }
    #
    # Takes the current event and matches it against the filter criteria
    # returns 1 for a match 0 for no match
    # Paramters:
    #   line - ASCII-zed event as a tcl list.
    #
    method matchFilter {line} {
	return 1;                                   # Stub.
    }
    #  Formats an event and adds it to the output window
    # Parameters:
    #    event   - ASCII-zed event as a tcl list.
    #
    method formatEvent event {
	$self add "---------------------"
	foreach word $event {
	    append line [format "%04x " $word]
	}
	$self add $line
    }
    
    # event - Called whenthe data source becomes readable.  This will happen
    #         either as a result of an event, in which case the event is read,
    #         or due to the data source being closed.
    # Parameters:
    #   The fd that was readable.
    #
    method event {f} {
	if {[eof $f]} {
	    close $f
	    return
	}
	gets $f line
	if {$sample} {
	    if {[$self matchFilter $line]} {
		$self formatEvent $line
		set sample 0
	    }
	}
    }
    #
    # requestSample - Request the next event that matches the filter criteria
    #                 be displayed on the text widget.
    #
    method requestSample {} {
	set sample 1
    }
}


mainwindow .w
pack .w








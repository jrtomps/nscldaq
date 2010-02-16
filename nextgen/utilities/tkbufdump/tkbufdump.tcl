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
source [file join $here itemdefs.tcl]
source [file join $here controlformatter.tcl]
source [file join $here unknownformatter.tcl]
source [file join $here physicscountformatter.tcl]
source [file join $here acceptall.tcl]


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
    variable filter
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

	$self configurelist $args
	
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
	
	# The filter starts out as the accept all filter
	
	set filter [AcceptAll %AUTO%]

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
    #  Returns a two item list of the item Type and the byte order
    # Parameters:
    #   The raw event.
    #
    method typeAndOrder event {
	# Figure out the byte ordering.  'order' will be 1 for big endian
	# 0 for little endian.  The type field will tell us the ordering.
	# if the type field non zero in word 2 it's little endian else
	# the type is in word 3 and the data are bigendian.
	
	set itemType [lindex $event 2]
	if {$itemType == 0} {
	    set order  1
	    set itemType [lindex $event 3]
	} else {
	    set order 0
	}
	return [list $itemType $order]
    }
    # Return a list that is the body of an event.
    #
    method getBody event {
	set body [lrange $event 4 end]
	return $body
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
	
	# Figure out the byte ordering.  'order' will be 1 for big endian
	# 0 for little endian.  The type field will tell us the ordering.
	# if the type field non zero in word 2 it's little endian else
	# the type is in word 3 and the data are bigendian.
	
	set typeAndOrder [$self typeAndOrder $event]
	set itemType [lindex $typeAndOrder 0]
	set order    [lindex $typeAndOrder 1]
	
	set body [$self getBody $event]
	
	#   Format depending on the type ranges and type:
	#
	if {[lsearch $::ControlItems $itemType] != -1} {
	    set formatter ControlFormatter
	} elseif {$itemType == $::PHYSICS_EVENT_COUNT} {
	    set formatter PhysicsCountFormatter
	} else 	{
	    set formatter UnknownFormatter
	}
	$formatter fmt -body $body -main $self -type $itemType -order $order
	fmt format
	fmt destroy
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
	if {$sample > 0} {
	    if {[$self matchFilter $line]} {
		$self formatEvent $line
		incr sample -1
	    }
	}
    }
    #
    # requestSample - Request the next event that matches the filter criteria
    #                 be displayed on the text widget.
    #
    method requestSample {} {
	incr sample
    }
}


mainwindow .w
pack .w








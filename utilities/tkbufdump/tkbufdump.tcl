#! /usr/bin/tclsh
set version "V1.0"
#  This program is a GUI event inspector.
#  It uses ringselector in sampling mode to accept events from an
#  event source.  The software free runs event acceptance and, when requested,
#  provides the next event from the stream.
#
package require Tk
package require snit
package require BWidget
package require Iwidgets

set here [file dirname [info script]]

source [file join $here datasource.tcl]
source [file join $here itemdefs.tcl]
source [file join $here controlformatter.tcl]
source [file join $here unknownformatter.tcl]
source [file join $here physicscountformatter.tcl]
source [file join $here acceptall.tcl]
source [file join $here typefilterdialog.tcl]
source [file join $here itemtypefilter.tcl]
source [file join $here scalerformatter.tcl]
source [file join $here stringlistformatter.tcl]
source [file join $here physicsformatter.tcl]

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

	foreach m [list File Filter] {
	    set $m [menu $menubar.m$m]
	    $menubar add cascade -label $m -menu $menubar.m$m
	}
	set Help [menu $menubar.help]
	$menubar add cascade -label Help -menu $menubar.help

	$File add command -label Open... -command [mymethod openSource]
	$File add separator
	$File add command -label {Packet Defs...} -command [mymethod readPackets]
	$File add command -label {Plugin ...}     -command [mymethod sourcePlugin]
	$File add separator
	$File add command -label Exit -command exit

	$Filter add command -label "Filter Types..." -command [mymethod typeFilter]
	$Filter add separator
	$Filter add command -label "Remove Filter"   -command [mymethod removeFilter]
	
	$Help add command -label About...  -command [mymethod about]
	$Help add command -label Topics... -command [mymethod helpTopics]
	
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
	ArrowButton $win.bottom.next -dir right -command [mymethod requestSample] \
		    -height 30 -width 30
	
	
	pack $win.bottom.statusbar -side left
	pack $win.bottom.next     -side right
	grid $win.bottom -sticky sew
	
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
	set lines [lindex [split [$win.dump index end] .] 0]
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
    
    # sourcePlugin - Called when File->Plugin.. is clicked. Prompts for the
    #                name of a plugin file and sources it in at the global level.
    #
    method sourcePlugin {} {
	set pluginFile [tk_getOpenFile	-defaultextension .tcl			\
					-filetypes [list			\
							[list {Plugin files} .plugin] \
							[list {Tcl Files}    .tcl]	\
							[list {Tk Files}     .tk]	\
							[list {All Files}	*]	\
						    ]				\
					-title {Choose plugin file}		\
			]
	# Cancel will leave us with an empty plugin file..
	# We also need to ensure the file is readable:
	
	if {$pluginFile ne ""} {
	    if {[file readable $pluginFile]} {
		uplevel #0 [list source $pluginFile]
	    } else {
		tk_messageBox	-icon	error					\
				-message "Unable to open $pluginFile for read"	\
				-title	"Plugin file open error"		\
				-type	ok
	    }
	}
    }
    
    # readPackets - Called when File->Packet Defs... is clicked.  Prompts for
    #               a packet definition file which is then added to the standard
    #               set of packets understood by the physics formatter.
    #
    method readPackets {} {
	set packetFile [tk_getOpenFile	-defaultextension .def  		\
					-filetypes [list			\
							[list {Packet Files} {.def}] \
							[list {All Files}    *	]] \
					-title {Choose packet definition file}]
	# Cancel will leave us with an empty packetFile:
	
	if {$packetFile ne ""} {
	    if {![file readable $packetFile]} {
		tk_messageBox	-icon error					\
				-message "Unable to open $packetFile for read" 	\
				-title "Packet file open error"			\
				-type   ok
	    } else {
		PhysicsFormatter readPacketDefinitionFile $packetFile
	    }
	}
    }
    
    #  openSource - This is called when the File->Open button is clicked.
    #               The user is prompted for the host and user name from which
    #               data will be taken (Modal)  The ringselector program
    #               will be started on a pipe with an fd handler established
    #               to accept data.
    #
    method openSource {} {
	dataSourcePrompt .p	-okcommand [list .p dismiss]		\
				-cancelcommand [list destroy .p]	\
				-helptext  [$self helpFile open]
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
	    
	    set url tcp://$host/$user
	    
	    set fd [open "|$::daqbin/ringselector --source=$url --formatted --sample=PHYSICS_EVENT" r]
	    fconfigure $fd -buffering line
	    fileevent $fd readable [mymethod event $fd]
	}
    }
    
    # emoveFilter - Delete the current filter and set it back to the
    #               default filter...which accepts everthing
    #
    method removeFilter {} {
	$self replaceFilter [AcceptAll %AUTO%]
    }
    #
    #  Prompt for and set the appropriate type filter.
    #  Type filters require the data item to be drawn from a specific set of item
    #  types.
    #
    method typeFilter {} {
	TypeFilterDialog .filterprompt 	-okcommand [list %W dismiss]  		\
					-cancelcommand [list destroy %W]	\
					-helptext [$self helpFile filter]
	.filterprompt modal
	
	# If not cancelled, the widget still exists.. process the filter.
	#
	
	if {[winfo exists .filterprompt]} {
	    set itemtexts [.filterprompt cget -itemtypes]
	    set itemnums [list]
	    foreach item $itemtexts {
		lappend itemnums [.filterprompt nameToId $item]
	    }
	    destroy .filterprompt
	    
	    # Create the new filter and let it replace the old one:
	    
	    set newFilter [ItemTypeFilter %AUTO% -itemtypes $itemnums]

	    $self replaceFilter $newFilter
	}
    }
    #   Displays the application "About" help.
    
    method about {} {
	tk_messageBox	-icon	info						\
			-title	"About tkdumper"				\
			-message "tkdumper $::version\nAuthor: Ron Fox\n(c) 2010 Copyright MSU Board of Trustees All Righs Reserved"
    }
    # Brings up the hyperhelp widget:
    
    method helpTopics {} {
	if {![winfo exists .onlineHelp]} {
	    iwidgets::hyperhelp .onlineHelp 	-helpdir 	$::here			\
						-topics		[list 	intro		\
									menus		\
									[list "Getting events" next.html]]
	    .onlineHelp showtopic intro
	    
	}
	wm deiconify .onlineHelp
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
		
		#  give a bit of a data handling hiatus to let things display
		#  necessary at high event rates.
		
		fileevent $f readable [list]
		after 100 [list fileevent $f readable [mymethod event $f]]
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
    #-------------------------------------------------------------------------
    #  Private utility functions
    #
    
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
	set tanda [$self typeAndOrder $line]
	set type [lindex $tanda 0]
	set body [$self getBody $line]
	
	return [$filter applyFilter $type $body]
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
	} elseif {$itemType == $::INCREMENTAL_SCALERS} {
	    set formatter ScalerFormatter
	} elseif {[lsearch $::StringListItems $itemType] != -1} {
	    set formatter StringListFormatter
	} elseif {$itemType == $::PHYSICS_EVENT} {
	    set formatter PhysicsFormatter
	} else 	{
	    set formatter UnknownFormatter
	}
	$formatter fmt -body $body -main $self -type $itemType -order $order
	fmt format
	fmt destroy
    }
    #
    #  Replace the current filter with the one passed in
    #
    method replaceFilter newFilter {
	if {$filter ne ""} {
	    $filter destroy
	}
	set filter $newFilter
    }
    #
    #  Return the help text associated with a topic.  The help files are
    #  located in $here topic.html
    #
    method helpFile topic {
	set filename [file join $::here $topic.html]
	if {[file readable $filename]} {
	    set f [open $filename r]
	    set helpText [read $f]
	    close $f
	    return $helpText
	} else {
	    return ""
	}
    }

}


mainwindow .w
pack .w








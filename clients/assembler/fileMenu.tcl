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

package provide evbFileMenu 1.0
package require Tk
package require snit


#
#  Snit type that populates and manages the event builder's GUI
#  file menu.
#  The file menu is populated with the following items:
#   New...   - Creates a new configuration.
#   Save...  - Save the existing configuration.
#   Open...  - Opens and reads in a saved configuration.
#   ----------------------------------------------------
#   Exit     - Exits the application.
#
#   The configuration is assumed to be a list in some
#   global variable (see OPTIONS) below.
#   A shadow copy is maintained so that the File menu
#   is able to prompt for confirmation if an existing,
#   unsaved configuration will be modified as the result of
#   an operation.
#
# OPTIONS:
#    -configvar     - Name of global variable with configuration.
#    -menu          - File menu widget path.
#

snit::type fileMenu {
    option  -configvar
    option  -menu

    variable lastSavedConfig
    variable lastFileDirectory .

    constructor args {
	$self configurelist $args

	global $options(-configvar)

	set menu $options(-menu)

	$menu add command -label New...  -command [mymethod OnNew]
	$menu add command -label Save... -command [mymethod OnSave]
	$menu add command -label Open... -command [mymethod OnOpen]
	$menu add separator
	$menu add command -label Exit... -command [mymethod OnExit]

    }

    # Support dynamically changing the configuration variable:
    # Changing the variable is assumed to have it up to date.
    # and saved (maybe a bad assumption).
    #
    onconfigure -configvar value {
	global $value
	set options(-configvar) $value
	set lastSavedConfig [set $value]
    }

    # Method to prompt to confirm the destruction of a modified configuration.
    # The message is of the form:
    #  "There are unsaved changes to the configuration.  You will lose thes
    #  change if you $ifyou.  Click ok to continue or cancel if you don't want to 
    #  proceed".
    #

    method ConfirmDestroy {ifyou} {
	set message "There are unsaved changes to the configuration.  "
	append message "You will lose these changes if you $ifyou.    "
	append message "Click ok to continue (discarding changes) or cancel if you don't "
	append message " want to proceed (do nothing)"

	return [tk_messageBox -title "Are you sure?" \
                              -icon warning          \
                              -type okcancel         \
  		              -message $message]
    }
    
    #  Process the New... menu item. If the configuration has changed, confirm first.
    #  The action is to set the lastSavedConfig and the -configvar to empty.
    #
    method OnNew {} {
	global $options(-configvar)
	set    config  [set $options(-configvar)]
	if {$lastSavedConfig ne $config} {
	    set goAhead [$self ConfirmDestroy "create a new configuration"]
	    if {$goAhead ne "ok"} return
	}
	set $options(-configvar) [list]
	set lastSavedConfig      [list]
    }
    # Process The Save command... The configuration is saved to file
    # as a set command. The lastSavedConfig is made to be the same as the configuration
    # so that operations that destroy the configuration won't need a prompt until the
    # config changes again.
    #
    method OnSave {} {
	global $options(-configvar)
	set    config  [set $options(-configvar)]

	set fileName [tk_getSaveFile  -defaultextension .evb         \
			               -initialdir $lastFileDirectory \
  			               -title {Chose File to write.}  \
		      -filetypes {
			  {{Event Buidler Config Files} {.evb}}       
			  {{TCL Scripts}                {.tcl}}
			  {{All Files}                   * }}]

	# Cancellation leaves the name of the file empty:

	if {$fileName eq ""} {
	    return
	}

	set lastFileDirectory [file dirname $fileName]

	# Save the file:

	set fd [open $fileName w]
	puts  $fd "set $options(-configvar) [list $config]"
	close $fd
	set lastSavedConfig $config
    }
    # Handles the open menu.  If necessary confirm that we will be
    # destroying a saved configuration.
    # Create a slave interpreter and have that source the config file.
    # we'll then pull the specific options(-configvar) from the 
    # slave interpreter and save it both in the options(-configvar) and
    # the lastSavedConfig variable.
    #
    method OnOpen {} {
	global $options(-configvar)
	set config [set $options(-configvar)]
	if {$config ne $lastSavedConfig} {
	    set goAhead [$self ConfirmDestroy "read in a new configuration"]
	    if {$goAhead ne "ok"} return
	}

	# Clear to Read the file.

	set fileName [tk_getOpenFile  -defaultextension .evb         \
			               -initialdir $lastFileDirectory \
  			               -title {Chose File to write.}  \
		      -filetypes {
			  {{Event Buidler Config Files} {.evb}}       
			  {{TCL Scripts}                {.tcl}}
			  {{All Files}                   * }}]
	if {$fileName eq ""} {
	    return
	}

	set lastFileDirectory [file dirname $fileName]


	set slave [ interp create -safe]
	$slave expose source

	if {[catch {$slave eval source $fileName} msg]} {
	    puts "sourc error $msg"
	}

	# Try to get the config variable from the file:

	if {[catch {$slave eval set $options(-configvar)} config]} {
	    
	    # Unable to fetch the option from the slave:
	    
	    tk_messageBox -type ok -icon error -title "Error in config file" \
		-message "$fileName is not a valid configuration file"

	    return
	}
	
	# Everything worked if we got here:


	puts "Got config: $config"
	set $options(-configvar) $config
	set lastSavedConfig      $config

	interp delete $slave
    }
    #  Processes the exit command.  Confirms this if the configuration has been
    #  modified since the last save, recovery or new.
    #
    method OnExit {} {
	global $options(-configvar)
	set config [set $options(-configvar)]
	if {$lastSavedConfig ne $config} {
	    set goAhead [$self ConfirmDestroy exit]
	    if {$goAhead ne "ok"} return
	}
	exit 0
    }
}
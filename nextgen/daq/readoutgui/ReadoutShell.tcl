#!/bin/sh

#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#

#   start wish \
exec wish  ${0} ${@}
#
#  Script which is a readout shell for the readout
#  GUI.
# Usage:
#     ReadoutShell.tcl host path
#

#  Setup the auto_path to include the installation root
#  dir:

set here [file dirname [info script]]
set libdir [file join $here .. TclLibs]
set bindir $here
#
# Canonicalize $here/../TclLibs
#
set wd [pwd]
cd $libdir
set libdir [pwd]
cd $wd
#   
#  Prepend to auto_path only if it's not already 
#  there.
#
if {[lsearch -exact $auto_path $libdir] == -1} {
    set auto_path [concat $here $libdir $auto_path]
}
package require Tk



namespace eval ReadougGUIPanel {}

package require ReadoutGui
package require Experiment
package require ExpFileSystem
package require Configuration
package require DAQParameters
package require InitializeConfiguration


set textWidgetHeight 24


proc Usage {} {
    puts "Usage:"
    puts "   ReadoutShell ?-host=host? ?-path=path?"
    puts "       host - Name of the host on which to run the Readout."
    puts "       path - Path to readout program to run."
    puts " If any of these are missing, attempts are made to fetch them from the"
    puts " experiment configuration, which is sourced in prior to processing parameters"
    exit
}
proc ExactPath {path} {
    set dir [pwd]
    cd $path
    set exact [pwd]
    cd $dir
    return $exact
}

#-------------------------- Dialogs: ------------------------------
#
#   Output a dialog to figure out what to do if there is no
#   stage are link.
# Returns:
#   0   - Run with event recording disabled.
#   1   - Exit (default).
#
proc NoStageArea {} {
    set message \
	"There is no link named ~/stagearea "
    append message \
	"The event recording software relies on this link to "
    append message \
	"tell it where to record data.  When this account was set up "
    append message \
	"you should have received an 'event area'.  "
    append message \
	"Until you have created a link to this event aread named "
    append message "~/stagearea, you will not "
    append message \
	"be able to record event data. "
    append message \
	" If you need help setting up this link please contact a "
    append message \
	"computer group staff member for assistance."

    return [tk_dialog .stagearea "No stagearea" \
		$message warning 1 Continue Exit]

}
#
#   Output a dialog to figure out what to do if stagarea is not a
#   link or does not point to an existing directory:
# Returns:
#    0  - Keep running but disable event recording.
#    1  - Exit (default).
#
proc NoStageAreaDirectory {} {
    set message \
	"I have found  ~/stagearea, however it is either not a "
    append message " symbolic link or does not "
    append message "point to a valid directory."
    append message \
	"The event recording software relies on this link to "
    append message \
	"tell it where to record data.  When this account was set up "
    append message \
	"you should have received an 'event area'.  "
     append message \
	"Until you have created a link to this event aread named "
    append message "~/stagearea, you will not "
    append message \
	"be able to record event data."
    append message \
	" If you need help setting up this link please contact a "
    append message \
	"computer group staff member for assistance."
    return [tk_dialog .stagearea "No stagearea directory" \
		$message warning 1 Continue Exit]


}
#
#  Output a dialog to figure out what to do if stageare is a link
#  to a directory to which we don't have write access.
# Returns:
#   0   - Keep running but diable event recording.
#   1   - Exit (default).
#
proc NoStageAreaWriteAccess {} {
    set message \
	"There is a  ~/stagearea link, however the directory it "
    append message \
	"points to is one in which you will not be able to create files."
    append message \
	"The event recording software relies on this link to "
    append message \
	"tell it where to record data.  When this account was set up "
    append message \
	"you should have received an 'event area'.  "
    append message \
	"Until you have created a link to this event aread named "
    append message "~/stagearea, you will not "
    append message \
	"be able to record event data."
    append message \
	" If you need help setting up this link please contact a "
    append message \
	"computer group staff member for assistance."
    return [tk_dialog .stagearea "No stagearea write access" \
		$message warning 1 Continue Exit]

}

#
#   Check the stagearea for various pathalogical conditions.
#   This function will either:
#   - Exit if there are problems with the stage area and
#     that's the user's choice.
#   - Set CanWrite to 0 if there are problems with the
#     stage area and the user chooses to continue.
#   - Return with no action if the stage area is fine.
#
proc CheckStageArea {} {
    global CanWrite
    set    salink "~/stagearea";	# Name of link file.

    #  Need a stage area:

    set type 0
    catch {set type [file type $salink]}

    if {![file exists $salink] && ($type == 0)} {
	if {[NoStageArea]} {
	    exit;			# User's choice.
	} else {
	    set CanWrite 0
	    return
	}
    }
    # The stage area exists.. but is it a link
    # that points to a directory?

    set target      0
    set targettype  0
    set type [file type $salink]
    if {$type == "link"} {
	set target [file readlink $salink]
    }
    if {$target != 0} {
	catch {set targettype [file type $target]}
    }
    if {$type != "link" || $targettype != "directory"} {
	if {[NoStageAreaDirectory]} {
	    exit
	} else {
	    set CanWrite 0
	    return
	}
    }
    # The directory the stage area points to must also
    # be writable:

    if {![file writable $target]} {
	if {[NoStageAreaWriteAccess]} {
	    exit
	} else {
	    set CanWrite 0
	    return
	}
    }
    # Everything is ok!!!
}

#
#  Experiment specific files are sourced from:
#    ~/ReadoutCallouts.tcl
#    ~/experiment/current/ReadoutCallouts.tcl
#    ./ReadoutCallouts.tcl  if . that isn't ~experiment/current or ~
#
#   Note that >all< such scripts are sourced.
#
#  The usual purpose of such scripts is to provide the user callouts:
#
#   OnBegin   - Called just prior to starting the run.
#   OnEnd     - Called just after a run ends.
#   OnPause   - Called just after a run pauses.
#   OnResume  - Called just before a run resumes
#   OnStart   - Called just after the readout program starts up, or is
#               restarted.
set dir ""
proc SourceExperimentFiles {} {
    global dir
    set    directories ""
    set calloutpath {~ ~/experiment/current}
    foreach dir $calloutpath {
	set file [glob -nocomplain $dir/ReadoutCallouts.tcl]
	if {$file != ""} {
	    lappend directories [ExactPath $dir]
	    uplevel #0 source $file
	}
    }
    set curdir [pwd]
    set dir $curdir
    set file [glob -nocomplain $curdir/ReadoutCallouts.tcl]
    if {[lsearch -exact $directories $curdir] == -1} {
	if {$file != ""} {
	    
	    uplevel #0 source $file
	}
    }
}

#
#    setup the configuration by processing
#    command switches and using them to override
#    the current configured values.
#
proc setupConfiguration arglist {
    global textWidgetHeight
    set nomonitor false
    foreach item $arglist {

	#  If we're running in a tcl server, 
	#  we need to discard the -p and -a switches.
	#  as they are for the Tcl server not us.



	set threechars [string range $item 0 2]
	if {($threechars eq "-pM") || ($threechars eq {-aR})} {
	    continue
	}

        set flagvalue [split $item =]
        set flag   [lindex $flagvalue 0]
        set value  [lindex $flagvalue 1]
        switch -exact -- $flag {
            -host {
		DAQParameters::sourceHostIs $value
             }
            -path {
               if {[catch {DAQParameters::readoutPathIs $value} msg]} {
		    if {$msg eq "DAQParameters::NotFound"} {
			set errorMsg "-path=$value : File could not be found"
		    } elseif {$msg eq "DAQParameters::NotExecutable"} {
			set errorMsg "-path=$value : File is not executable"
		    } else {
			set errorMsg "-path=$value : Unanticipated error processing this flag"
		    }
		    tk_messageBox -icon error -type ok -title {Error in -host} \
			-message $errorMsg
		    exit
		}

            }
	    -nomonitor {
		set ::nomonitor true
	    }
            -ftphost {
		#
		#  NOTE -ftp is obsolete!!!
                DAQParameters::ftpHostIs $value
		puts stderr "You are using the -ftp switch which is no longer needed"
            }
            -passwd {
                DAQParameters::passwordIs $value
            }
	    -textheight {
		set textWidgetHeight $value
	    }
            default {
                Usage
                exit -1
            }
        }
    }
    puts "At the end of all this we have $textWidgetHeight"

}
# getPassword
#    Prompt graphically for missing passwords.
#
#
proc getPassword {} {
    global Password
    wm withdraw .

    toplevel .password
    label .password.prompt -text Password:
    entry .password.entry -show *
    frame .password.actions -relief groove -borderwidth 3
    button .password.actions.ok -text Ok -command {
        set Password [.password.entry get]
        destroy .password
    }
    button .password.actions.cancel -text Cancel -command {
        set Password ""
        destroy .password
    }
    # Lay it out.

    pack .password.actions.ok .password.actions.cancel -side left
    pack .password.prompt .password.entry .password.actions -side top -fill x

    # Make it modal..

    grab set .password
    tkwait window .password


    wm deiconify .

    return $Password
}

#  Set up the configuration subsystem

InitializeConfiguration::addSubsystem DAQParameters
InitializeConfiguration::addSubsystem ReadoutState
InitializeConfiguration::addSubsystem ExpFileSystemConfig

InitializeConfiguration::initialize;   # Define packages and defaults...
InitializeConfiguration::processEnv;   # Override defaults with env.


#
#  Configuration files include:
#    ~/.daqconfig
#    ./.daqconfig
#    These get added to the config set and configured in
#    Once that's done the ExpFileSystem is used to locate
#    the configuration file that might have been written by
#    the last time we ran in: file join [ExpFileSystem::GetStage] .readoutconfig
#
set homeconfig [file join ~ .daqconfig]
set cwdconfig  [file join . .daqconfig]

if {[file exists $homeconfig]} {
    InitializeConfiguration::addConfigFile $homeconfig
}
if {[file exists $cwdconfig]} {
    InitializeConfiguration::addConfigFile $cwdconfig
}

set sessionconfig [file join [ExpFileSystem::GetStage] .readoutconfig]
if {[file exists $sessionconfig]} {
    InitializeConfiguration::addConfigFile $sessionconfig
}

# Process the configuration files and then override again with the env:

InitializeConfiguration::processConfigFiles
InitializeConfiguration::processEnv

set CanWrite 1;				# By default events can be recorded.

# Read the extensions and check the stage area.

SourceExperimentFiles
CheckStageArea

# Process command language overrides.

setupConfiguration $argv

# If necessary get a password via a prompt.

#if {[DAQParameters::getPassword] == ""} {
#    set password [getPassword]
#    if {$password == ""} {
#        tk_messageBox -icon error \
#            -message {No password given, attempts to record data will fail} \
#           -title {missing password} -type ok
#  } else {
#        DAQParameters::passwordIs $password
#    }
#}

if {$CanWrite} {
    ExpFileSystem::Convert
    ExpFileSystem::CreateHierarchy
    Experiment::CleanOrphans
}




# Register the experiment and start the
# readoutgui.

Experiment::Register
ReadoutGui::ReadoutController {}


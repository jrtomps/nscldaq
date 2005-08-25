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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

# (C) Copyright Michigan State University 1937, All rights reserved
# $Header$
# Stager.tcl
#   Main procedures for the stager.
#   Note that all tapes are written via the rsh package to allow acces
#   to remote tapes.  We assume the unix systems accessed by rsh are
#   sharing filesystems.
#


package provide  Stager 2.0

package require  ExpFileSystem
package require  StagerConfiguration
package require  rsh
package require  RunRetension
package require  Wait
package require Diagnostics
namespace eval   Stager {
    variable ArchiveCommand "tar chvf "
    variable Tardone 0
}
# Stager::dirsToRuns dirs
#      Converts a set of run directories to a
#      list of run numbers.
#
proc Stager::dirsToRuns dirs {
    set runs ""
    foreach dir $dirs {
	set dirTail [file tail $dir]
	if {[scan $dirTail run%d run] == 1} {
	    lappend runs $run
	}
    }
    return $runs
}


# Stager::tapeSerial
#     Return the next tape serial number.
#
proc Stager::tapeSerial {} {
    set SerialNo [StagerConfiguration::getTapeNumber]
    return $SerialNo

}
# Stager::cleanCommandFiles
#       cleans up from past runs by killing off old
#       command files.
#
proc Stager::cleanCommandFiles {} {
    set Root [ExpFileSystem::GetStage]
    set oldfiles [glob -nocomplain [file join $Root .stagecommand*]]
    foreach file $oldfiles {
	file delete $file
    }
}


# Stager::GenerateStageList
#    Returns a list of the top level directories that need to be staged.
#    This is a list of all directories that have not yet been staged.
#
proc Stager::GenerateStageList {} {
    set metaDir [ExpFileSystem::WhereisMetadata]
    set runs [glob -nocomplain $metaDir/run*]
    set stageList ""
    foreach run $runs {
	set taildir [file tail $run]
	if {[scan $taildir "run%d" runNumber] == 1} {
	    if {[lsearch -exact [StagerConfiguration::getStageList] $runNumber] == -1} {
		#
	        # note below, a file which is a link to a dir will show up
		# in [file isdirectory as true]
		#
	        if {[file isdirectory $run] && [catch {file readlink $run}]} {
		    lappend stageList $run
		}
	    }
	}
    }
    return $stageList

}
# Stager::TarLogInput
#  Event processor for input data from tar pipeline:
#
# Parameters:
#   fd       - Channel open on the pipe we are monitoring
#   callback - User callback we invoke when there's data.
proc Stager::TarLogInput {fd callback} {
    if {[eof $fd]} {
	while {1} {
	    set line [read $fd 1000]
	    if {[string length $line] == 0 } {
	        incr ::Stager::Tardone
	        catch {close $fd}
	        return
	    } else {
	        $callback $line
	    }
	}
    } else {
	set line [read $fd 1000]  ;# Get some data...
        $callback $line
    }
}
# Stager::WriteToTape
#  Writes a list of files in the 'ROOT' directory to tape.
#
#   The callback script is called each time a line comes in from
#   The archive command. By default, the puts command receives this line
#   which causes the output to go to stdout.
# Parameters:
#     files      - List of full paths to files (directories) to tar off.
#     callback   - Callback to invoke on user input.
#

proc Stager::WriteToTape {files callback} {
    variable ArchiveCommand
    variable SerialNo

    set RemoteDrive [StagerConfiguration::TapeDrive]
    set RemoteHost  [StagerConfiguration::TapeHost]

    set ftails ""
#	puts "Writing $files"
    foreach file $files {
	lappend ftails [file tail $file]
    }
    set SerialNo [Stager::tapeSerial]
    StagerConfiguration::incrTapeNumber

# Clean up any old stager command scripts:

    Stager::cleanCommandFiles

    set Root [ExpFileSystem::GetStage]
    set stagescript [open [file join $Root .stagecommand$SerialNo] {WRONLY CREAT} \
		0775]
#	puts "Stagerscript cd $Root"
    puts $stagescript "cd $Root"

    append RemoteDrive [StagerConfiguration::tapeHostIs] : \
			   [StagerConfiguration::tapeDriveIs]
    set    cmd $ArchiveCommand
    append cmd  $RemoteDrive " " $ftails

#	puts "Stagerscript $cmd"
    puts $stagescript $cmd
    close $stagescript
#	puts "Stagerscript closed:"
#	exec cat $Root/.stagecommand


    set info [rsh::rshpid $RemoteHost [file join Root .stagecommand$SerialNo]]
    set tarpid [lindex $info 0]
    set tarin  [lindex $info 2]
    close $tarin
    set mytarout [lindex $info 1]


    fconfigure $mytarout -buffering line -blocking 0
    fileevent $mytarout readable \
		"Stager::TarLogInput $mytarout $callback"
    vwait Stager::Tardone
    flush stdout
    set status [Wait -pid $tarpid]  ;# Reap status.
    flush stdout
    return [lindex $status 1]

}
# Stager::DeleteEventData files
#  Delete event data
# Parameters:
#    files -  is the set of directories which contain the metadata
#              and the event data. These are complete paths.
#  Returns:
#   a list of files which were not deleted because they had
#   retensions pending.
#
proc Stager::DeleteEventData {files} {
    set retained ""
    foreach file $files {
	set tail [file tail $file]
        scan $tail "run%d" run
	set evtfiles [glob -nocomplain [file join $file *.evt]]

	if {![ExpRunRetension::isPending $run]} {
	    if {$evtfiles != ""} {
		foreach evtfile $evtfiles {
		    if {[catch {file readlink $evtfile}]} {
			catch {file delete -force $evtfile}
		    } else {
			lappend retained $evtfile
		    }
		}
	    }
	} else {
	    set retained [concat $retained $evtfiles]
	}
    }
    return $retained
}

#  Stager::MoveRetainedData files
#     Takes the run directories in the list and
#     moves them into the retained area.  A link to these
#     directories is created in the experiment dir.
#
proc Stager::MoveRetainedData {files} {
    set destDir [ExpFileSystem::WhereareRetainedEventFiles]
    foreach file $files {
	set srcDirname [file tail $file]
	scan $srcDirname run%d runNumber
	if {[catch {file readlink $file}]} {  ; # file is not a link
	    set fullDestName [file join $destDir $srcDirname]
	    file rename $file $fullDestName
	    exec ln -s $fullDestName $file
	    ExpRunRetension::MoveToRetained $runNumber
	} else { ;# file is a link.
	    Diagnostics::Warning "The file $file is already a link meaning it's been moved or staged already"
	}
    }
}
# Stager::MoveMetaData dirs
#   Moves the run data into the staged subdir.
#   A link is made in the experiment
#
proc Stager::MoveMetaData {dirs} {
    set destDir [ExpFileSystem::WhereisStagedMetaData]

    foreach dir $dirs {
	set dirName [file tail $dir]
	if {[catch {file readlink $dir}]} {;        # not a link
	    set fullDestName [file join $destDir $dirName]
	    file rename $dir $fullDestName
	    exec ln -s   $fullDestName $dir
	} else {;                                   # a link.
	    Diagnostics::Warning "The file $dir is already a link meaning it's been staged or retained already"
	}
    }
}

#  Initiate the staging.
#

proc Stager::Stage {} {
    #
    # Migrate files to tape.
    #
    set StageFiles [Stager::GenerateStageList]
    if {[llength $StageFiles] == 0} {
	return
    }
    Stager::WriteToTape      $StageFiles
    set Retained [Stager::DeleteEventData  $StageFiles]
    Stager::MoveRetainedData $Retained
    Stager::MoveMetaData     $StageFiles

    # The stage list and the retained list are all lists of
    # directory paths, while the staged and retained list
    # expect run numbers.  We need to update both of these
    # lists in the configuration:

    set stagedRuns [dirsToRuns $StageFiles]
    StagerConfiguration::appendStageList $stagedRuns

    set retainedRuns [dirsToRuns $Retained]
    StagerConfiguration::appendRetainList $retainedRuns
}
namespace eval Stager {
    namespace export SetHost SetDrive Initialize Stage
    namespace export WriteToTape
    namespace export DeleteEventData
    namespace export MoveRetainedData
    namespace export MoveMetaData
    namespace export GenerateStageList

}


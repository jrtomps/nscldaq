#
#   package to support burning a DVD set for nscl daq.  
#
#  

package provide DvdBurn 1.0

namespace eval  DvdBurn {
    #
    #   Package/namespace global variables:
    #
    variable ExperimentRoot "~/experiment"; # Root of experiment files.
    variable ISOFSroot      "/scratch/$tcl_platform(user)/isos"; # Where ISO temp data lives.
    variable MarshallDir    $ISOFSroot/tmp; # Where runs get marshalled.

    variable DVDSize        4000;	# Mbytes in a DVD.
    #
    # Exported functions:
    #
    namespace export EnumerateRuns;	# List the set of runs.
    namespace export FilterList;	# Filter a tcl list.
    namespace export MarshallRun;	# Marshall a run to a temp directory
    namespace export CreateDVDDirectory; # Create root of dvd filesystem.
    namespace export RunToDVD;		# Transfer a marshalled run to dvd-fs.
    namespace export CreateISO;		# Create an iso from a dvd-fs tree.
    namespace export BurnDVD;		# Burn a single dvd from an iso.
    namespace export CreateDvds;	# Burn a dvd set.
    namespace export LowRunFilter;	# Low cutoff filter predicate for runs
    namespace export HighRunFilter;	# High cutoff filter predicate for runs

    
    # Procedure:    DirectoryToRun
    # Purpose:      Translate a directory name to a runnumber.
    # Inputs:       A directory.
    # Outputs:      The number of the run that corresponds to the directory.
    # Errors:       If the directory does not correspond to a run number.
    #               e.g. ~/experiment/george.
    # Description:  The last path element of the directory is matched
    #               against patterns of the form run<run-number>
    #               The run number is decoded and returned.
    #
    proc DirectoryToRun {directory} {
	#
	#  Ensure the final element of the directory path is of the form.
	#  run<n> where n is a number, and extract the run number.
	#
	set SubDirectory [file tail $directory];
	if {![regexp  {^run([0-9]+)$} $SubDirectory]} {
	    error "DirectoryToRun: No  match for run<number> in $SubDirectory"
	}
	if {[scan $SubDirectory "run%d" RunNumber] != 1} {

	    error "DirectoryToRun: Scan for run number failed $SubDirectory"
	}
	return $RunNumber
    }
    # Procedure:   CompareRuns
    # Purpose:     Compare a pair of run number directory files.
    # Inputs:      dir1    - full path to the first run directory.
    #              dir2    - Full path to the second run directory.
    # Outputs:     -1   The run number for dir1 is < the run number for dir2.
    #              0    The run number for dir1 is == the run number for dir2.
    #              1    The run number for dir1 is > the run number for dir2.
    # Description: For both directories, DirectoryToRun is called to convert
    #              the directories to run numbers.  If successful, the runs
    #              are compared as described in Outputs above. Failures
    #              are handled as follows:
    #                failure on dir1 returns 1 -> pushing it towards the list
    #                        end in an lsort.
    #                failur on dir2 returns -1 -> Pushing it towards the list
    #                        end in an lsort.
    #
    proc CompareRuns {dir1 dir2} {
	if {[catch "DirectoryToRun $dir1" run1]} {
	    return 1
	}
	if {[catch "DirectoryToRun $dir2" run2]} {
	    return -1
	}
	if {$run1 < $run2} {
	    return -1
	}
	if {$run1 > $run2} {
	    return 1
	}
	return 0

    }
    
    # Procedure:	LowRunFilter
    # Purpose:	Predicate that filters run directory lists with a low level 
    #               cutoff.
    # Inputs:	Run directory name (full path).Low cutoff
    # Outputs:	� True if the run number of the directory is >=  to the 
    #                 run number specified by the predicate argument.
    #           � False if run number is < run number specified by predicate 
    #                 argument.
    # Errors:	NONE
    # Description:	Extracts the run number associated with the run 
    #               directory name and returns true if this is >= to the 
    #               predicate argument. Note that directories that don't 
    #               decode to run numbers result in a false without even
    #               a whimper of an error message.
    #
    proc LowRunFilter {directory cutoff} {
	if { [catch "DirectoryToRun $directory" RunNumber]} {
	    return 0
	}
	return [expr $RunNumber >= $cutoff]
    }
    # Procedure:	HighRunFilter
    # Purpose:	Predicate that filters run directory lists with a high 
    #           levelcutoff.
    # Inputs:	Run directory name (fullpath)High cutoff
    # Outputs:	� True if the run number of the directory is <= the high
    #             cutoff parameters
    #           � False if the run number of the directory is > the high 
    #             cutoff parameter.
    # Errors:	NONE
    # Description:	Filter predicate that can be used to produce a 
    #                   list of the run directories that corresponds to 
    #                   runs <= a particular run number.
    proc HighRunFilter {directory cutoff} {
	if {[catch "DirectoryToRun $directory" RunNumber]} {
	    return 0
	}
	return [expr $RunNumber <= $cutoff]
    }

    # Procedure:	FilterList (reuse lib).
    # Purpose:	        Reduce a list according to some filter criteria
    # Inputs:	        List - An arbitrary TCL list.
    #                   Predicate - see description below, the predicate 
    #                               returns true for each element of the 
    #                               list that is acceptable in the output list.
    #                   Predicate arg  - Parameter passed without 
    #                               interpretation to the predicate.
    # Outputs:	        The set of list elements that passed the predicate's 
    #                   test.
    # Errors:	        None.
    # Description:	Each element of the list is passed to the predicate 
    #                   member function.  The predicate returns true if the 
    #                   element is acceptable for the output list, and false 
    #                   otherwise.  The predicate is called as follows:
    #                        Predicate element arg
    #                  Where element is a complete entry that appears in the 
    #                  input list. and arg is the predicate argument passed 
    #                  into FilterList.  The predicate function can do 
    #                  arbitrary processing.
    proc FilterList {List Predicate PredicateArgument} {
	set OutputList ""
	foreach element $List {
	    if {[$Predicate $element $PredicateArgument] } {
		lappend OutputList $element
	    }
	}
	return $OutputList
    }

    # Procedure:	EnumerateRuns
    # Purpose:	        Return a list of all of the runs that exist in an 
    #                   experiment.
    # Inputs:	        Experiment top level directory (e.g. ~/experiment).
    # Outputs:	        A TCL list of run directory names e.g. {run1 run2 run3}
    # Errors:	        Error if toplevel directory does not exist.  Note that
    #                   run list can be empty.
    # Description:	Determines the set of run directories that exist below
    #                   the top level directory specified.  The set of run 
    #                   directory names is sorted by the numeric run number. 
    #                   (not e.g. run1 run10 run11).  The full path of each 
    #                   directory is returned (e.g. /user/03901/run1 not run1).
    proc EnumerateRuns {TopDirectory} {
	set RunList [glob -nocomplain $TopDirectory/run*]; # Raw enumeration.

	return [lsort -command CompareRuns $RunList]; # Sorted enumeration.
    }

    # Procedure:	MarshallRun
    # Purpose:	        Copies all the files that will be recorded for a run 
    #                   to a specified target directory.
    # Inputs:	        Run number              - Number of the run to marshall
    #                   Source experiment root  - Where the run dirs live.
    #                   Target directory.       - Destination of the marshal.
    # Outputs:	        Number of Mbytes of data in the run (floating point).
    # Errors:	        Any errors in the copy operation.
    # Description:	The target directory is removed if it exists and a 
    #                   new one created.  The following files are copied into
    #                   that directory:
    #                   � All files in the source run directory with links 
    #                     followed. This copy is a deep (hierarchical) copy.
    #                   � A spectra directory is created and the spectra 
    #                     associated with that run are copied from
    #                     root/../savedspectra
    #                   � A scaler directory is created and the scaler files 
    #                     associated with the run are copied from 
    #                     root/../scalerfiles to it.
    #
    proc MarshallRun {Run SourceRoot Target} {
	#
	#   If the target exists, delete it.
	#   Failure to delete is an error.
	if {[file exists $Target]} {
	    if {[catch "file delete -force $Target"]} {
		error "MarshallRun: Unable to remove an existing $Target"
	    }
	}
	#  Make the new target:
	#
	if {[catch "file mkdir $Target"]} {
	    error "MarshallRun: Could not create marshalling target $Target"
	}
	#  Compute the directory that holds the run and ensure it exists:
	#
	set Source $SourceRoot/run$Run
	if {![file exists $Source]} {
	    error "MarshallRun: Source directory $Source does not exist"
	}

	#  tar the source directory to the target:

	set wd [pwd]
	cd  $Source
	if {[catch "exec tar czhf - . | tar xzfC - $Target" msg]} {
	    cd $wd
	    error "MarshallRun: Bulk data tar failed: $msg"
	}

	cd $wd;				# Restore working directory.

	# Size the result with du and return the size of the result.
	#  size will be either
	#     nnU
	#  nn a floating point and U a unit (K, M or G).
	#  or:
	#    nn  (in which case the units are K).
	#
	set size [exec du -sh $Target]
	set n [scan $size "%f%s" size units]
	if {$n == 1} {			# Units are K.
	    return [expr $size/1000] ;	# Return mBytes.
	} else {
	    switch $units {
		K { return [expr $size/1000] }
		M { return $size }
		G { return $size*1000 }
		default { 
		    error "MarshallRun: Unrecognized size units: $units" }
	    }
	}
	error "MarshallRun: >>BUG<<< control should not have reached here"
    }
    # Procedure:	CreateDVDDirectory
    # Purpose:	        Create a top level directory in which a dvd file 
    #                   image will be created.
    # Inputs:	        DVDRoot - a top level directory in which the file 
    #                             systems are being created.
    #                   DVDNumber- Volume number within the dvd set.
    # Outputs:	        Full path to the dvd directory.
    # Errors:	        If mkdir fails.
    # Description:	Creates a directory, into which the file system of a 
    #                   dvd can be created. (named Root/diskn where n is the 
    #                   dvd volume number).
    proc CreateDVDDirectory {DVDRoot DVDNumber} {
	
	# Figure out the directory name:

	set Directory $DVDRoot/Disk$DVDNumber

	#  Create the directory...

	if {[catch "file mkdir $Directory"]} {
	    error "CreateDVDDirectory - unable to create $Directory"
	}

	return $Directory
    }
 
    # Procedure:	RunToDVD
    # Purpose:	        Move a marshaled run into a dvd filesystem image
    # Inputs:	        Source directory
    #                   DVD volume root (from CreateDVDDirectory)
    #                   Run Number
    # Outputs:	        NONE
    # Errors:	        Errors from the move.
    # Description:	The source directory is mv'd to the runrunnumber 
    #                   under the DVD volume root specified.
    proc RunToDVD {Source DVDRoot Run} {
	
	# Figure out the target:
	
	set ResultingDirectory $DVDRoot/run$Run

	# If this directory already exists we need to remove it or
	# we'll get a nested directory...

	if {[catch "file delete -force $ResultingDirectory"]} {
	    error "RunToDVD Unable to remove pre-existing $ResultingDirectory"
	}

	# Now we can do the mv.

	if {[catch "file rename $Source $ResultingDirectory"]} {
	    error "RunToDVD Unable to rename $Source -> $ResultingDirectory"
	}
    }

    # Procedure:        FdToStdout
    # Purpose:          Copy all input from a filedescriptor until EOF to
    #                   stdout.  Flushes are done after all writes
    #                   Just in case stdout is e.g. a pipe.
    # Inputs:           file
    # Outputs            -> Stdout.
    # Errors:           NONE:
    # Description:      Opens the designated fifo
    #                   Reads until eof, writing each line on stdout.
    #                   Close the fifo.
    proc FdToStdout {fifo} {



	set fd [open |cat $fifo r]
	puts [gets $fd]
	while {![eof $fd]} {
	    puts [gets $fd]
	    flush stdout
	}
	close $fd
    }

    # Procedure:        Fifo
    # Purpose:          Create a fifo special file, destroying an existing one
    # Inputs:           path to the fifo.
    #
    proc Fifo {fifo} {
	file delete -force $fifo
	exec mkfifo $fifo

	return $fifo
    }

    # Procedure:	CreateISO
    # Purpose:	Create an ISO image of a dvd file image.
    # Inputs:	DVDVolume root (from CreateDVDDirectory)
    #               Desired output file.
    # Outputs:	
    # Errors:	
    # Description:	Runs mkisofs to create the desired dvd ISOimage from 
    #                   the specified DVDVolume root.
    
    proc CreateISO {DVDRoot isoname} {
	
	set fifo [Fifo .mkisofs]

	exec cat $fifo &
	
	exec mkisofs  -r  -J -v -o $isoname $DVDRoot > $fifo  2> $fifo 

	
    }
    
    # Procedure:	BurnDVD
    # Purpose:	        Burn a dvd from an ISO Image.
    # Inputs:	        ISO filename.
    # Outputs:	        None
    # Errors:	
    #  Description:	Interacts with the DVD Burn programs to burn a dvd 
    #                   from the specified ISO image.
    proc BurnDVD {iso} {
	set fifo [Fifo .burndvd]
	exec cat $fifo &
	exec burnDVD $iso > $fifo 2> $fifo
	


    }
    # Procedure:	CreateDvds
    # Purpose:	        Create the set of dvds that spans the runs desired.
    # Inputs:	        Run-range [optional]  If present, the range of 
    #                   runs desired.
    # Outputs:	        NONE
    # Errors:	
    # Description:	Creates all of the DVDs needed to save the run files 
    #                   that need to be saved for the specified runs.  If 
    #                   supplied, run-range will specify a subset of runs to
    #                   save.  Valid specifications for run-range are:Low   
    #                   All runs beginning with run number Low and ending with
    #                   the highest numbered run taken are saved.Low-High All 
    #                   runs beginning with run number Low and ending with 
    #                   (and including) run High are saved.

    proc CreateDvds {{range {}}} {
	variable ExperimentRoot
	variable ISOFSroot
	variable DVDSize
	variable MarshallDir

	file delete -force $ISOFSroot;	# Get rid of any pre-existing stuff.

	# Enumerate and if necessary filter the run set so that we
	# have the runs that can be burned in RunList:

	set RunList [EnumerateRuns $ExperimentRoot]
	if {[llength $range] != 0} {	#  Must filter:
	    set low [lindex $range 0]
	    set RunList [FilterList $RunList LowRunFilter $low]

	    set hi [lindex $range 1] 
	    if {$hi != ""} {
		set RunList [FilterList $RunList HighRunFilter $hi]
	    }
	}

	if {[llength $RunList] == 0} {
	    puts "There are no runs in the specified range"
	    exit -2
	}
	#  Now Build the DVD file systems 
	
	set volume 1;			# DVD number.
	set volroot($volume) [CreateDVDDirectory $ISOFSroot $volume]
	set SizeLeft $DVDSize
	
	foreach run $RunList {
	    set runnum [DirectoryToRun $run]
	    puts "Marshalling run $runnum $SizeLeft MB remaining."
	    set RunSize [MarshallRun $runnum $ExperimentRoot $MarshallDir]
	    if {$RunSize > $SizeLeft} {
		puts " run $runnum does not fit... starting new filesystem root"
		incr volume
		set  volroot($volume) [CreateDVDDirectory $ISOFSroot $volume]
		set SizeLeft $DVDSize
	    }
	    if {$RunSize > $SizeLeft} {
		error " run $runnum is bigger than a DVD!!!"
	    }
	    puts " Moving run $runnum ($RunSize MB) to $volroot($volume): "
	    RunToDVD $MarshallDir $volroot($volume) $runnum
	    set SizeLeft [expr $SizeLeft - $RunSize]

	}
	puts " All runs are marshalled. Now we make $volume  ISO images: "
	for  {set i 1} {$i <= $volume} {incr i} {
	    CreateISO $volroot($i) $ISOFSroot/iso$i.iso
	    puts "Finished with volume $i"
	}

	puts "Now we will burn $volume DVD(s)"
	for {set i 1} {$i <= $volume} {incr i} {
	    puts "BURNDVD: Put volume $i in the burner, hit <ENTER> to burn"
	    gets stdin
	    BurnDVD $ISOFSroot/iso$i.iso
	}
    }

}

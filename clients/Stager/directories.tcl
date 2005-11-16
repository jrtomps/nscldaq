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


package provide ExpFileSystem 2.0
package require ExpFileSystemConfig
package require DAQParameters
package require Diagnostics

namespace eval ExpFileSystem {
    namespace export WhereareRetainedEventFiles
    namespace export WhereareCompleteEventFiles
    namespace export WhereisCurrentEventData GenRunFileBase
    namespace export WhereisRunFile GenRunFile WhereisRun
    namespace export WhereisCurrentData GetStage
    namespace export CreateHierarchy
    namespace export WhereisMetadata
    namespace export WhereisStagedMetaData
}


# ExpFileSystem::CreateHierarchy
#
#      Create the entire directory hierarchy.  All Directories
#      are created with mkdir -p so that paths to them will be made
#      if needed and no errors will be reported if the dirs arleady
#      exist
#
#
proc ExpFileSystem::CreateHierarchy {}  {
    # Fetch the location of the stagearea.  In this implementation
    # We truly don't care if this is a link or a dir.. we are just
    # going to create the needed directories inside of it.
    # If the user puts his/her stuff on the same filesystem as
    # their home directory, that may be useful e.g. for tests.
    #
    #   This proc is unix specific.

    set stagearea [ExpFileSystemConfig::getStageArea]


    file mkdir [file join $stagearea current]
    file mkdir [file join $stagearea complete]
    file mkdir [file join [ExpFileSystem::WhereisMetadata] current]
    file mkdir [file join $stagearea retained]
    file mkdir [file join $stagearea staged]

    #  Now make the link to it, if it does not already exist.

    set expdir [ExpFileSystemConfig::getExperimentDirectory]
    if {[file exists $expdir]} {
        if {[catch {file readlink $expdir}]} {
            set msg "$expdir already exists and is not a link\n"
            Diagnostics::Warning $msg
            error {ExpFileSystem::CreateHierarchy Conflicting existing files in the way}
        }
    } else {
        catch {exec ln -s [file join [ExpFileSystem::WhereisMetadata]] $expdir}
    }
}
    # GetStage
    #    Returns the directory of the stagearea.
    #
proc ExpFileSystem::GetStage {} {
    return [ExpFileSystemConfig::getStageArea]
}
    # WhereisCurrentData
    #    Returns the directory that holds the current run metadata
    #
proc ExpFileSystem::WhereisCurrentData {} {
    set expdir [ExpFileSystem::WhereisMetadata]
    return [file join $expdir current]
}
    # WhereisRun
    #    Returns the directory that holds the current run.
    #    This is the run[mmm] directory in the experiment dir.
    #
proc ExpFileSystem::WhereisRun {num} {
    set expdir [ExpFileSystem::WhereisMetadata]
    append subdirname run $num
    return [file join $expdir $subdirname]
}
#  GenRunFileBase
#     Returns the basename of a run file.
#     A run filename has the form:
#         [format run%d_%d-%d.evt  run segment buffersize]
#     We're going to return the run%d part of this.
#
proc ExpFileSystem::GenRunFileBase {num} {
    return [format run%d $num]
}
# GenRunFile
#    Generate the full name of a run file.
# Parameters:
#    num   - Run number.
#    seq   - The sequence number of the file.
# NOTES:
#    See GenRunFileBase for a discussion of the form of
#    these files.
#
proc ExpFileSystem::GenRunFile {num {seq 0}} {

    set fname [GenRunFileBase $num]
    set buffer [DAQParameters::getBufferSize]
    set buffer [expr {$buffer/2}]

    #  The form of the name depends on the sequence:

    if {$seq} {
        set fname [format %s_%d-%d.evt $fname $seq $buffer]
    } else {
        set fname [format %s-%d.evt $fname $buffer]
    }
    return $fname
}
# WhereisRunFile
#    Return the full path to a selected event file
#    The assumption is that the run has already ended.
#    This implies that the run file is now in the experiment/run
#    directory.
# Parameters:
#  num  - The run number we are interested in.
#  seq  - The sequence number of the file within the run.
#
proc ExpFileSystem::WhereisRunFile {num {seq 0}} {
    set expdir [ExpFileSystem::WhereisMetadata]
    set subdir [format run%d $num]
    set filename [ExpFileSystem::GenRunFile $num $seq]
    return [file join $expdir $subdir $filename]
}

# WhereisCurrentEventData
#     Returns the directory in which the current run file can be found.
#
proc ExpFileSystem::WhereisCurrentEventData {} {
    set eventdir [ExpFileSystemConfig::getStageArea]
    return [file join $eventdir current]

}
# WhereareCompleteEventFiles
#    Returns the name of the directory in which the current
#    event files can be found.  Note that this is
#    a deprecated function as the run files are now in the
#    experiment area.  For compatibility, the stagearea/complete
#    directory is maintained containing links to all the event files.
#
proc ExpFileSystem::WhereareCompleteEventFiles {} {
    set stage [ExpFileSystemConfig::getStageArea]
    return [file join $stage complete]
}
# WhereareRetainedEventFiles
#    This retuns the location of the retained event files.
#    On a stage pass, it is possible to designate event files
#    as retainable.  These files, when the stage pass is complete
#    are moved to the directory described by this proc.
#
proc ExpFileSystem::WhereareRetainedEventFiles {} {
    set stage [ExpFileSystemConfig::getStageArea]
    return [file join $stage retained]

}
# WherisMetadata:
#   Returns the top level metadata directory path
#
proc ExpFileSystem::WhereisMetadata {} {
    return [file join [ExpFileSystemConfig::getStageArea] experiment]
}
# WhereisStagedMetaData:
#    When a run is staged, it is moved to the
#    staged directory so that it is clear that
#    it is no longer elligible for staging.
#    This proc describes where that is.
#
proc ExpFileSystem::WhereisStagedMetaData {} {
    set stagedir [ExpFileSystemConfig::getStageArea]
    return [file join $stagedir staged]
}

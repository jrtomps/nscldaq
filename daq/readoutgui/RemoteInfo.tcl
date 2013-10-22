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


# (C) Copyright Michigan State University 1937, All rights reserved 
#
#  Package to get information about tape drives on remote systems
#
#  
package provide RemoteInfo 1.0
package require ssh
namespace eval RemoteInfo  {
#                             OS   Vers Tape Dir    Spec
    variable TapeDriveSpecs {{Linux {} /dev/        nst?}  
                            {OSF1 V5.1 /dev/ntape/   *d0}
                            {OSF1 V5.0 /dev/ntape/  *c} 
                            {OSF1 V4.0 /dev/        nrmt*h} }

   #  Returns the last line of a multiline list.
   #  This is used/needed for people who output stuff in their
   #  login files.
   #
   proc LastLine {line} {
       set lines [split $line '\n']
       return [lindex $lines end]
   }
   #
   #  The drive list may have stuff prepended to it, so we only return
   #  lines which match the stuff we want.
   #
   proc SelectDrives {list mask} {
       set lines [split $list '\n']
       set drives ""
       foreach line $lines {
	   if {[string match $mask $line]} {
	       lappend drives $line
	   }
       }
       return $drives
   }
#      Return name of operating system.
#
   proc GetOs {host} {
       set Os [ssh::ssh $host uname]       
       return [LastLine $Os]
   }
#
#     Return version number of OS release.
   proc GetOsVsn {host} {
       set Version [ssh::ssh $host "uname -r"]
       return [LastLine $Version]
   }
#
#    Given an Operating system specification record returns
#    what GetTapeListMask would need.
#
   proc MergeSpec {OsSpec} {
       set dir [lindex $OsSpec 2]
       append mask [lindex $OsSpec 3]
       set     Spec $dir
       append  Spec $mask
       return $Spec
  }
#
#    Given an OS and it's release level,return the thing to tack on
#    an ls command on that system to list all tape special device files
#    available.
#
#      /dev/nst*
#
   proc GetTapeListMask {os release} {
       variable TapeDriveSpecs
       foreach OsSpec $TapeDriveSpecs  {
	   if { $os == [lindex $OsSpec 0] } {
	       if { [lindex $OsSpec 1] == ""} {
		   return [MergeSpec $OsSpec]
	       }
	       if { $release == [lindex $OsSpec 1]} {
		   return [MergeSpec $OsSpec]
	       }
	   }
       }
       error "GetTapeListMask - Unsupported OS/Version $os/$release"

   }
#
#    Get List of tape drives on remote system.
#
   proc GetTapeDrives {host} {
       set OS [GetOs $host]
       set V  [GetOsVsn $host]
       set mask [GetTapeListMask $OS $V]
       set command ls
       lappend command $mask
       set drives [ssh::ssh $host $command]
       return [SelectDrives $drives $mask]
   }
   namespace export GetOs GetOsVsn GetTapeListMask GetTapeDrives
}



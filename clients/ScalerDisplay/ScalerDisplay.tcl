# Meant to be run on Tclserver.
# 

puts  "---------------------------------"


puts  $argc
puts  $argv


#  We require DAQHOST to be an environment
#  variable that is the name of the spdaq system.
#

if {[array names env DAQHOST] == ""} {
	tk_dialog .failure "No DAQ HOST" \
	"The DAQHOST environment variable must be set to the spdaq system you're using" \
	error 0 Dismiss
	exit -1
}
set spdaq  $env(DAQHOST)
set spdaqurl tcp://$spdaq:2602/


#  The user must have supplied a command line argument 
#  which is the name of their setup file.
#

if {[llength $argv] < 2} {
   tk_dialog .failure "No setup file" \
   "You must supply a command argument that is your scaler config file" \
   error 0 Dismiss
   exit -1
}
set file [lindex $argv 0]

#  And the file must be readable...
#
if {![file readable $file]} {
	tk_dialog .failure "Unreadable setup file" \
	"The setup file $file  is not readable by me and must be" \
	error 0 Dismiss
	exit -1
}


#  We need to establish our location and source in the
#  scaler GUI support. We assume that we are installed
#  1 level below 'prefix' and that the scaler display
#  GUI support packages are installed in the Scripts
#  dir of that directory.

set me [info script]
set mydirectory [file dirname $me]
set bindir     $mydirectory/../bin
set scriptdir  $mydirectory/../TclLibs/ScalerDisplay
source $scriptdir/scaler.tcl

# Canonicalize the TclLibs directory subtree location and
# if needed, prepend to auto_path.


set libDir [file join  $mydirectory ..]
set wd [pwd]
cd $libDir
set libDir [pwd]
cd $wd

if {[lsearch -exact $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}


#
#   Now the setup file is sourced we can start the scalerclient
#   and set things up so that we will kill it on our exit:
#

puts "exec $bindir/sclclient -p $TclServerPort  -s $spdaqurl "
 
set clientpid [exec $bindir/sclclient -p $TclServerPort  -s $spdaqurl & ]

proc Cleanup {widget pid} {
    if {$widget != "."} return
    exec kill $pid
}

bind . <Destroy> "Cleanup %W $clientpid"



#
#  Source the user's setup file:
#

if {[catch "source $file" msg]} {
   tk_dialog .failure "Setup file failure" \
   "There is an error in the setup file $argv : $msg" \
   error 0 Dismiss
   exit -1
}

#!/bin/sh

# start wish \
exec wish ${0} ${@}
set ChannelCount 16



set me $argv0                       ;# Full path to this script.
set instdir [file dirname $me]      ;# Where we are installed.

source $instdir/shaperapp.tcl

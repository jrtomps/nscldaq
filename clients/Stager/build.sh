#!/bin/csh
setenv OS `uname`                  # operating system for conditional comps.

if ( $#argv ==  1 ) then
   set instdir = $1
else if ( $#argv == 0 ) then
   echo -n "Full path to installation directory: "
   set instdir = $<
else
   echo "Usage: "
   echo "    build.sh  [installation_directory_full_path]"
   exit
endif


echo "Installing in" $instdir
mkdir -p $instdir/Scripts

install -m 0775 *.tcl $instdir/Scripts
install -m 0775  RunReadout.sh $instdir/Scripts

rm -rf $instdir/Bin/Stager
ln -s $instdir/Scripts/StagerGui.tcl $instdir/Bin/Stager

./installpkg $instdir

echo "Stager installed"

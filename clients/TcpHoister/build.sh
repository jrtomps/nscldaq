#!/bin/csh
setenv OS `uname`                  # operating system for conditional comps.
umask 02

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

echo "Building directory tree skeleton:"

mkdir -p $instdir/contrib

make INSTDIR=$instdir OS=$OS
make install INSTDIR=$instdir OS=$OS

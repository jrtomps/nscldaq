#!/bin/csh
setenv OS `uname`                  # operating system for conditional comps.
umask 002
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

if ( -e /dev/vme24d32 ) then    # There's no vendor lib for nscl look for dev.
   setenv VMEDEV NSCLBIT3
endif

if ( -e $instdir/Include/btdef.h ) then # This works if no hardware installed.
   setenv VMEDEV SBSBIT3
endif




echo "Installing in" $instdir

make clean
make depend
make -k OS=$OS INSTDIR=$instdir VMEDEV=$VMEDEV
make install OS=$OS INSTDIR=$instdir


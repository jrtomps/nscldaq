#!/bin/csh
setenv OS `uname`
if ( $#argv == 1 ) then
    setenv instdir  $1
else if  ( $#argv == 0 ) then
    echo  -n "Installation directory:  "
    setenv instdir  $<
else
    echo Usage:
    echo " build.sh [install_dir]
endif

# Bit 3 can have one of 2 drivers; The
#  nscl driver derived from Nikhef's work and
#  the SBS/Bit3 driver itself.
#
if ( -e /dev/vme24d32 ) then    # There's no vendor lib for nscl look for dev.
   setenv VMEDEV NSCLBIT3
endif

if ( -e $instdir/Include/btdef.h ) then # This works if no hardware installed.
   setenv VMEDEV SBSBIT3
endif




echo Vme device is $VMEDEV
echo "Installing in " $instdir
mkdir -p $instdir/Scripts
mkdir -p $instdir/Lib
mkdir -p $instdir/Include
mkdir -p $instdir/include

echo Building Utility base:

(cd SpecTcl/Utility; make install OS=$OS INSTDIR=$instdir)

echo Building Exception library:

(cd SpecTcl/Exception;make clean;  make install OS=$OS INSTDIR=$instdir)

echo Building Tcl++ class library: 

(cd SpecTcl/TCL;make clean; make install OS=$OS INSTDIR=$instdir)

echo Building dynamically loadable vme command package:

make clean
make -- VMEDEV=$VMEDEV OS=$OS INSTDIR=$instdir depend
make -- VMEDEV=$VMEDEV OS=$OS INSTDIR=$instdir system
make -- VMEDEV=$VMEDEV OS=$OS INSTDIR=$instdir install






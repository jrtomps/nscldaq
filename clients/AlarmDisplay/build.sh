#!/bin/csh
setenv OS 'uname'                 # operating system

if ( $#argv == 1 ) then
    set instdir = $1
else if ( $#argv == 0 ) then
    echo -n "Full path to installation directory: "
    set instdir = $<
else
    echo "Usage: "
    echo "    build.sh [installation_directory_full_path]"
    exit
endif


echo "Installing in" $instdir

install -d -m 02775 -p $instdir/Scripts
install -d -m 02775 -p $instdir/Bin
install -d -m 02775 -p $instdir/Lib
install -d -m 02775 -p $instdir/Include


install -m 0775 *.tcl $instdir/Scripts
install -m 0775 AlarmDisplay $instdir/Scripts


ln -s $instdir/Scripts/AlarmDisplay $instdir/Bin
install -m 0664 *.h $instdir/Include

(./installpkg $instdir)

echo Building Libraries

make INSTDIR=$instdir libMonitoring.a 
install -m 0664 libMonitoring.a $instdir/Lib

echo Building Alarm Server:

make INSTDIR=$instdir 
make INSTDIR=$instdir install

echo Installation completed successfully







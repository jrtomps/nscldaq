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

install -d -m 02775  $instdir/Scripts
install -d -m 02775  $instdir/Bin
install -d -m 02775  $instdir/Images

install -m 0775  *.tcl $instdir/Scripts
install -m 0775  LogDisplay $instdir/Bin
install -m 0775  *.gif $instdir/Images

./installpkg $instdir

echo "Installation completed successfully"

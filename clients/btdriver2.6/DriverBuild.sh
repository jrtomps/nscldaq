#!/bin/bash
#
#   Initiates a build of the device driver for the 2.6 kernel.
#
#   First need to locate the kernel sources.  For now we support two styles
#   of kernel source directory locations:
#
#    Debian:  /usr/src/kernel-source-<version>
#    Redhat:  /usr/src/linux-<version>
#

version="$(uname -r | cut -f1 -d-)"
Debiansrc="/usr/src/kernel-source-${version}"
RedHatsrc="/usr/src/linux-${version}"

#  Figure out which it is:

if [ -d $Debiansrc ] 
then
Linuxsrc="$Debiansrc"
fi

if [ -d $RedHatsrc ]
then
Linuxsrc="RedHatsrc"
fi

BTDRIVER=$(pwd)

pushd dd
make -C $Linuxsrc SUBDIRS=$(pwd) modules BTDRIVER=${BTDRIVER}



popd

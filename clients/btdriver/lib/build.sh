#/bin/bash
#
#   Build the bit 3 device driver and install the files where the NSCL
#   daq expects them to be.
#
dest=$1
export dest
echo building in $dest
umask 002

(
    umask 002
    cd sys
    make config
    (umask 002
     make)
    (
     umask 002

       
       cd ..
       (
         umask 002
	 cd lib; make 
       )

       install -d -m 02775 $dest/daq/Include
       install -d -m 02775 $dest/daq/Lib
       install -d -m 02775 /lib/modules/$(uname -r)/kernel/drivers/misc
       find . -name "*.h" -exec install -m 0774  {} $dest/daq/Include \;
       install -m 0774 lib/*.a $dest/daq/Lib
       install -m 0774 dd/btp.o /lib/modules/`uname -r`/kernel/drivers/misc
       /lib/modules/mkbtp 1
       
       echo "##### NSCL specific device drivers" >> /etc/modules.conf
       echo "# btp - SBS/Bit3 device driver."    >> /etc/modules.conf
       echo alias "/dev/btp*" /dev/btp >>/etc/modules.conf
       echo probe /dev/btp  btp >> /etc/modules.conf
       echo alias char-major-$(cat /proc/devices|grep btp |cut -f1 -d' ') btp >> /etc/modules.conf

    )


)

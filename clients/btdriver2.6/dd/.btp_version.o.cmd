cmd_/daq/clients/btdriver2.6/dd/btp_version.o := gcc -Wp,-MD,/daq/clients/btdriver2.6/dd/.btp_version.o.d -nostdinc -iwithprefix include -D__KERNEL__ -Iinclude  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -pipe -msoft-float -mpreferred-stack-boundary=2  -march=i686 -Iinclude/asm-i386/mach-default -O2 -fomit-frame-pointer  -g -DDEBUG  -DBT1003 -I/daq/clients/btdriver2.6/dd -I/daq/clients/btdriver2.6/include -fno-strength-reduce -Wall -Wstrict-prototypes -fno-common  -DMODULE -DKBUILD_BASENAME=btp_version -DKBUILD_MODNAME=btp -c -o /daq/clients/btdriver2.6/dd/.tmp_btp_version.o /daq/clients/btdriver2.6/dd/btp_version.c

deps_/daq/clients/btdriver2.6/dd/btp_version.o := \
  /daq/clients/btdriver2.6/dd/btp_version.c \

/daq/clients/btdriver2.6/dd/btp_version.o: $(deps_/daq/clients/btdriver2.6/dd/btp_version.o)

$(deps_/daq/clients/btdriver2.6/dd/btp_version.o):

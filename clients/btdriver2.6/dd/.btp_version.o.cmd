cmd_/root/btdriver/dd/btp_version.o := gcc -Wp,-MD,/root/btdriver/dd/.btp_version.o.d -nostdinc -iwithprefix include -D__KERNEL__ -Iinclude  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -pipe -msoft-float -mpreferred-stack-boundary=2  -march=i686 -Iinclude/asm-i386/mach-default -O2 -fomit-frame-pointer  -g -DDEBUG  -DBT1003 -I/root/btdriver/dd -I/root/btdriver/include -fno-strength-reduce -Wall -Wstrict-prototypes -fno-common  -DMODULE -DKBUILD_BASENAME=btp_version -DKBUILD_MODNAME=btp -c -o /root/btdriver/dd/.tmp_btp_version.o /root/btdriver/dd/btp_version.c

deps_/root/btdriver/dd/btp_version.o := \
  /root/btdriver/dd/btp_version.c \

/root/btdriver/dd/btp_version.o: $(deps_/root/btdriver/dd/btp_version.o)

$(deps_/root/btdriver/dd/btp_version.o):

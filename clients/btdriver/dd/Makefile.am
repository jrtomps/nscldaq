SUFFIXES =  .c .o
.c.o:
	$(CC) -c $(INCLUDES) $<

OBJECTS= btp_cfg.o btp_open.o btp_rdwr.o btp_mmap.o btp_ioctl.o \
    btp_lowio.o btp_version.o btp_bind.o \
    bt_mem.o bt_mutex.o bt_event.o bt_bit.o bt_llist.o \
    bt_delay.o bt_rwlck.o \
    bt_icbr_thread.o bt_isr.o bt_nbus.o bt_param.o bt_setup.o \
    bt_xfer.o

EXTRA_DIST=btp_cfg.c btp_open.c btp_rdwr.c btp_mmap.c btp_ioctl.c \
    btp_lowio.c btp_version.c btp_bind.c \
    bt_mem.c bt_mutex.c bt_event.c bt_bit.c bt_llist.c bt_delay.c \
    bt_rwlck.c \
    bt_icbr_thread.c bt_isr.c bt_nbus.c bt_param.c bt_setup.c \
    bt_xfer.c btdd.h btpdd.h mkbtp

all-local: btp.o


btp.o: $(OBJECTS)
	$(LD) -r -o btp.o $(OBJECTS)


INCLUDES= -I. -I../include $(SMP_FLAG) -fno-strength-reduce \
	-g -DDEBUG -DBT1003 -O2 $(KERNEL_INCS) \
	-D__KERNEL__ -DMODULE -DEXPORT_SYMTAB \
	-Wall -Wstrict-prototypes


install-data-local:
	install -m 0744 btp.o \
	         /lib/modules
	./mkbtp 1       # Make first unit for him.
	echo "##### Bit 3 drivers:" >> /etc/modules.conf
	echo alias "/dev/btp*" /dev/btp >>/etc/modules.conf
	echo probe /dev/btp btp >> /etc/modules.conf
	echo alias char-major-$(cat /proc/devices |grep btp | \
                                cut -f1 -d' ') btp >> \
				/etc/modules.conf
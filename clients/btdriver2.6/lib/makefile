#!/bin/csh -f
#
#	Filename:	makefile
#
#	Purpose:	Compile the Linux PCI/Nanobus Mirror API library
#
#	Copyright (c) 1999 by SBS Technologies, Inc.
#	All rights reserved.
#
#	$Revision$
#

EXTRA_DEFINE=-DBT1003 
ADDED_CFLAGS=-g -DDEBUG -D_GNU_SOURCE 
#ADDED_CFLAGS=-DNDEBUG -DBT_INLINE
CC_OPTIM=-O2

BTINCLUDE=../include

#CPPFLAGS =-I$(BTINCLUDE) -D__KERNEL__ -DMODULE -DEXPORT_SYMTAB $(EXTRA_DEFINE)
CPPFLAGS =-I$(BTINCLUDE) $(EXTRA_DEFINE)

CFLAGS=$(CC_OPTIM) -I. -I$(BTINCLUDE) $(ADDED_CFLAGS) \
       -Wall -Wstrict-prototypes \
	-D_POSIX_C_SOURCE -D_POSIX_THREAD_SEMANTICS

# Solaris needs:	`getconf LFS64_CFLAGS` -D__EXTENSIONS__ -mt \

OBJECTS= bt_rdwr.o bt_ctrl.o bt_mmap.o bt_open.o bt_bind.o \
	 bt_chke.o bt_clre.o bt_init.o bt_perr.o bt_locking.o bt_info.o \
	 bt_name.o bt_serr.o bt_icbr.o  bt_reset.o bt_string.o \
	 bt_ior.o bt_iow.o bt_stat.o bt_cas.o bt_tas.o bt_send_irq.o bt_or_io.o \
	 bt_hw_diag.o


BT_SHFILE=$(BTINCLUDE)/btio.h $(BTINCLUDE)/btpio.h $(BTINCLUDE)/btdef.h \
	$(BTINCLUDE)/btpdef.h $(BTINCLUDE)/btngpci.h $(BTINCLUDE)/btapi.h \
	$(BTINCLUDE)/btpapi.h

BT_HFILE=$(BT_SHFILE) btpiflib.h

default:	libbtp.a

clean:
	rm -f *.o *.a 

$(OBJECTS): %.o: %.c $(BT_HFILE)
	-@$(RM) $@
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

libbtp.a: $(OBJECTS)
	$(AR) -r $@ $?
	cp $@ $(BTINCLUDE)


.FAILED:
	@echo ""


SUFFIXES =  .c .o
.c.o:
	$(CC) -c $(INCLUDES) $<

OBJECTS= askpci.o fops.o plxbug.o

EXTRA_DIST=askpci.c fops.c plxbug.c \
	askpci.h common.h fops.h main.h pciif.h pcivme.h plx9050.h \
	plxbug.h vic.h vme.h


all-local: pcivme.o


pcivme.o: $(OBJECTS)
	$(LD) -r -o pcivme.o  $(OBJECTS)


INCLUDES= -I. -I../include $(SMP_FLAG) -fno-strength-reduce \
	-g -DDEBUG  -O2 $(KERNEL_INCS) \
	-D__KERNEL__ -DMODULE -DEXPORT_SYMTAB \
	-Wall -Wstrict-prototypes


install-data-local:
install-data-local:
	echo Wiener PCI/VME bus bridge driver 
	echo Copyright "(c)" 2000-2002 ARW Electronik Germany
	echo Redistributed under their GPL license.
	install -m 0744 pcivme.o \
	         /lib/modules
	install -m 0744 pcivme_load /lib/modules

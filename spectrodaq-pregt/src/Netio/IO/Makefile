
FILES = sockopts udpio tcpio unixio

H_FILES = netio_parms.h sockopts.h udpio.h tcpio.h unixio.h netio.h

include ../../../config.make

all: include lib

include::
	$(DIRINST) $(NETIOINCDIR)
	$(INSTALL) $(H_FILES) $(NETIOINCDIR)

lib: $(O_FILES)
	$(AR) $(NETIOLIB) $(O_FILES)

install: $(O_FILES)
	$(DIRINST) $(INST_NETIOINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_NETIOINCDIR)
	$(AR) $(INST_NETIOLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend	

include .depend

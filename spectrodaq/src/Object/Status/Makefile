
H_FILES = DAQStatus.h

FILES = DAQStatus

include ../../../config.make

all: include lib prog

include::
	$(CP) $(H_FILES) $(DAQINCDIR)

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend

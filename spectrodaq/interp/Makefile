
SUB_DIRS = 

FILES = DAQInterp DAQInterpChannel DAQCommOutChannel DAQInterpCommand \
DAQDisplayOutChannel

H_FILES = DAQInterp.h DAQInterpChannel.h DAQCommOutChannel.h \
DAQInterpCommand.h  DAQDisplayOutChannel.h

PROG_FILES = 

include ../config.make

all: include lib prog

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
#	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib: $(O_FILES)
	$(AR) $(SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog: $(PROG_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make install); done;

clean::
	rm -f *.o
#	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
	rm -f $(PROG_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend

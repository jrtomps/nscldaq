
SUB_DIRS = NodeContext

H_FILES = DAQContext.h

FILES = DAQContext

include ../../../config.make

all: include lib prog

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

prog::
	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)
	for i in $(SUB_DIRS); do (cd $$i; make install); done;

clean::
	rm -f *.o
	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend

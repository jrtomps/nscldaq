
SUB_DIRS = Pthreads

FILES = 

H_FILES = daqthreads.h

include ../../config.make

all: include lib

include::
	$(DIRINST) $(DAQTHREADSINCDIR)
	$(INSTALL) $(H_FILES) $(DAQTHREADSINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib: $(O_FILES)
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

install: $(O_FILES)
	$(DIRINST) $(INST_DAQTHREADSINCDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQTHREADSINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make install); done;

prog::

clean::
	rm -f *.o
	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend

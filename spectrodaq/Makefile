
SUBDIRS = nscl xmloop src scripts interp server tools tests

PROPERDIRS = nscl xmloop src interp server tools

H_FILES = daqconfig.h

include config.make

SUB_DIRS = $(SUBDIRS) $(iface_subdir)

PROPER_DIRS = $(PROPERDIRS) $(iface_subdir)

all: include lib prog examples

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib::
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog::
	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

toys::
	(cd src; make toys)

install: include lib prog
	$(DIRINST) $(INST_DAQINCDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make install); done;
	(cd examples/; make -f Makefile.install install)
	(cd doc/; make install)

examples::
	(cd examples/; make -f Makefile.install)

depend::
	for i in $(SUB_DIRS); do (cd $$i; make depend); done;

clean::
	for i in $(SUB_DIRS); do (cd $$i; make clean); done;
	(cd examples/; make -f Makefile.install clean)
	rm -f *.o

realclean: clean
	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;
	(cd examples/; make -f Makefile.install realclean)
	rm -f config.cache config.make config.log config.h config.status

proper: clean
	for i in $(PROPER_DIRS); do (cd $$i; make realclean); done;
	for i in $(PROPER_DIRS); do (cd $$i; make depend); done;

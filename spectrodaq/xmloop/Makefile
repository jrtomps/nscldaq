
SUB_DIRS = expat xmloop

FILES = 

H_FILES = xmlpconfig.h

include ./config.make

all: pre include lib 

pre::
	for i in $(SUB_DIRS); do (cd $$i; make pre); done;

include::
	$(DIRINST) $(XMLINCDIR)
	$(INSTALL) $(H_FILES) $(XMLINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib::
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

install::
	$(DIRINST) $(INST_XMLINCDIR)
	$(INSTALL) $(H_FILES) $(INST_XMLINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make install); done;

prog::

clean::
	rm -f *.o
	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
	rm -f include/*
	rm -f lib/*
	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend
# DO NOT DELETE

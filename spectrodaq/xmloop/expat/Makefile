
SUB_DIRS = gennmtab xmltok xmlparse

FILES = 

H_FILES = 

include ../config.make

all: pre include lib 

pre::
	for i in $(SUB_DIRS); do (cd $$i; make pre); done;

include::
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib::
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

install::
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
# DO NOT DELETE

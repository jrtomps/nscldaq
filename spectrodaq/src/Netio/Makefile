
SUB_DIRS = IO 

FILES = 

H_FILES = 

include ../../config.make

all: include lib

include::
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib: $(O_FILES)
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

install: $(O_FILES)
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

FILES =

H_FILES = 

SUB_DIRS = exceptions

include ../../config.make

all: include lib prog

lib::
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

include::
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

prog::
	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

install::
	for i in $(SUB_DIRS); do (cd $$i; make install); done;

clean:
	rm -f *.o 
	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend

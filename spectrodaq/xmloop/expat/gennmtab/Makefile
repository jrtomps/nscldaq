
FILES = gennmtab

H_FILES = 

include ../../config.make

all: include

include: gennmtab.c
	$(CXX) $(CFLAGS) -o gennmtab gennmtab.c

lib::

install::

clean::
	rm -f *.o
	rm -f gennmtab

realclean: clean
	rm -f .depend
	:> .depend	

include .depend


SUB_DIRS = 

FILES = Interp

H_FILES = Interp.h

PROG_FILES = sdaqcons

include ../config.make

CXXFLAGS = $(FLAGS) $(tcl_cflags)
CFLAGS = $(FLAGS) $(tcl_cflags)

all: include lib prog

include::
#	$(INSTALL) $(H_FILES) $(DAQINCDIR)
#	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib::
#	$(AR) $(SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog: $(PROG_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

sdaqcons: $(O_FILES) sdaqcons.cc
	$(CXX) $(CXXFLAGS) -o sdaqcons sdaqcons.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(THREADLIBS) $(LDFLAGS) $(tcl_libs)

install: $(O_FILES)
	$(DIRINST) $(INST_BINDIR)
	$(BININST) $(PROG_FILES) $(INST_BINDIR)
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

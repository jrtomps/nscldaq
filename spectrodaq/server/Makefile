
SUB_DIRS = 

FILES = 

H_FILES = 

PROG_FILES = spectrodaq

ETC_FILES = spectrodaq.conf

include ../config.make

all: include lib prog

include::
#	$(INSTALL) $(H_FILES) $(DAQINCDIR)
#	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib::
#	$(AR) $(SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog: spectrodaq.cc $(O_FILES)
	$(CXX) $(CXXFLAGS) -o spectrodaq spectrodaq.cc $(O_FILES) $(NETIOLIB) $(SERVLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(tcl_libs) $(THREADLIBS) $(CRYPTLIBS) $(LDFLAGS)
#	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

install: $(O_FILES)
	$(DIRINST) $(INST_BINDIR)
	$(DIRINST) $(INST_CONFDIR)
	$(BININST) $(PROG_FILES) $(INST_BINDIR)
	$(INSTALL) $(ETC_FILES) $(INST_CONFDIR)
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

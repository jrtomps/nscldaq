
SUB_DIRS = 

FILES = 

H_FILES = 

PROG_FILES = spectrodaq

ETC_FILES = spectrodaq.conf

include ../config.make

all: include lib prog

include::

lib::

prog: spectrodaq.cc $(O_FILES)
	$(CXX) $(CXXFLAGS) -o spectrodaq spectrodaq.cc $(O_FILES) $(NETIOLIB) $(SERVLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(tcl_libs) $(THREADLIBS) $(CRYPTLIBS) $(LDFLAGS) $(NETIOLIB)

install: $(O_FILES)
	$(DIRINST) $(INST_BINDIR)
	$(DIRINST) $(INST_CONFDIR)
	$(BININST) $(PROG_FILES) $(INST_BINDIR)
	$(INSTALL) $(ETC_FILES) $(INST_CONFDIR)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend
	rm -f $(PROG_FILES)

include .depend


SUB_DIRS = 

FILES = 

H_FILES = 

PROG_FILES = objstat tsstat linkstat addlink dellink usedpages debuglevel \
daqtrack prqstat servstat freepages loopback logstate

include ../config.make

all: include lib prog

include::
#	$(INSTALL) $(H_FILES) $(DAQINCDIR)
#	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib::
#	$(AR) $(SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog: $(PROG_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

logstate: $(O_FILES) logstate.cc
	$(CXX) $(CXXFLAGS) -o logstate logstate.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

loopback: $(O_FILES) loopback.cc
	$(CXX) $(CXXFLAGS) -o loopback loopback.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqtrack: $(O_FILES) daqtrack.cc
	$(CXX) $(CXXFLAGS) -o daqtrack daqtrack.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

objstat: $(O_FILES) objstat.cc
	$(CXX) $(CXXFLAGS) -o objstat objstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

tsstat: $(O_FILES) tsstat.cc
	$(CXX) $(CXXFLAGS) -o tsstat tsstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

linkstat: $(O_FILES) linkstat.cc
	$(CXX) $(CXXFLAGS) -o linkstat linkstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

servstat: $(O_FILES) servstat.cc
	$(CXX) $(CXXFLAGS) -o servstat servstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

usedpages: $(O_FILES) usedpages.cc
	$(CXX) $(CXXFLAGS) -o usedpages usedpages.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

freepages: $(O_FILES) freepages.cc
	$(CXX) $(CXXFLAGS) -o freepages freepages.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

prqstat: $(O_FILES) prqstat.cc
	$(CXX) $(CXXFLAGS) -o prqstat prqstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

debuglevel: $(O_FILES) debuglevel.cc
	$(CXX) $(CXXFLAGS) -o debuglevel debuglevel.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

addlink: $(O_FILES) addlink.cc
	$(CXX) $(CXXFLAGS) -o addlink addlink.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(SERVLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

dellink: $(O_FILES) dellink.cc
	$(CXX) $(CXXFLAGS) -o dellink dellink.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(SERVLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

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

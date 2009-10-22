
SUB_DIRS = 

FILES = 

H_FILES = 

PROG_FILES = objstat tsstat linkstat addlink dellink usedpages debuglevel \
daqtrack prqstat servstat freepages loopback logstate

include ../config.make

all: include lib prog

include::

lib::

prog: $(PROG_FILES)

logstate: $(O_FILES) logstate.cc
	$(CXX) $(CXXFLAGS) -o logstate logstate.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS) $(NETIOLIB)

loopback: $(O_FILES) loopback.cc
	$(CXX) $(CXXFLAGS) -o loopback loopback.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

daqtrack: $(O_FILES) daqtrack.cc
	$(CXX) $(CXXFLAGS) -o daqtrack daqtrack.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

objstat: $(O_FILES) objstat.cc
	$(CXX) $(CXXFLAGS) -o objstat objstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

tsstat: $(O_FILES) tsstat.cc
	$(CXX) $(CXXFLAGS) -o tsstat tsstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

linkstat: $(O_FILES) linkstat.cc
	$(CXX) $(CXXFLAGS) -o linkstat linkstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

servstat: $(O_FILES) servstat.cc
	$(CXX) $(CXXFLAGS) -o servstat servstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

usedpages: $(O_FILES) usedpages.cc
	$(CXX) $(CXXFLAGS) -o usedpages usedpages.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

freepages: $(O_FILES) freepages.cc
	$(CXX) $(CXXFLAGS) -o freepages freepages.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

prqstat: $(O_FILES) prqstat.cc
	$(CXX) $(CXXFLAGS) -o prqstat prqstat.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

debuglevel: $(O_FILES) debuglevel.cc
	$(CXX) $(CXXFLAGS) -o debuglevel debuglevel.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

addlink: $(O_FILES) addlink.cc
	$(CXX) $(CXXFLAGS) -o addlink addlink.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(SERVLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

dellink: $(O_FILES) dellink.cc
	$(CXX) $(CXXFLAGS) -o dellink dellink.cc $(O_FILES) $(NETIOLIB) $(CLIENTLIB) $(SERVLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)  $(NETIOLIB)

install: $(O_FILES)
	$(DIRINST) $(INST_BINDIR)
	$(BININST) $(PROG_FILES) $(INST_BINDIR)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend
	rm -f $(PROG_FILES)

include .depend

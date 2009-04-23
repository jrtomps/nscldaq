
SUB_DIRS = 

FILES = 

H_FILES = 

PROG_FILES = tsstat linkstat addlink dellink usedpages debuglevel \
daqtrack prqstat servstat freepages logstate

TOOL_LIBS =  $(O_FILES) $(DAQLIB) $(CLIENTLIB) $(NETIOLIB) \
$(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS)

include ../config.make

all: include lib prog

include::

lib::

prog: $(PROG_FILES)

logstate: $(O_FILES) logstate.cc
	$(CXX) $(CXXFLAGS) -o logstate logstate.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

daqtrack: $(O_FILES) daqtrack.cc
	$(CXX) $(CXXFLAGS) -o daqtrack daqtrack.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

tsstat: $(O_FILES) tsstat.cc
	$(CXX) $(CXXFLAGS) -o tsstat tsstat.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

linkstat: $(O_FILES) linkstat.cc
	$(CXX) $(CXXFLAGS) -o linkstat linkstat.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

servstat: $(O_FILES) servstat.cc
	$(CXX) $(CXXFLAGS) -o servstat servstat.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

usedpages: $(O_FILES) usedpages.cc
	$(CXX) $(CXXFLAGS) -o usedpages usedpages.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

freepages: $(O_FILES) freepages.cc
	$(CXX) $(CXXFLAGS) -o freepages freepages.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

prqstat: $(O_FILES) prqstat.cc
	$(CXX) $(CXXFLAGS) -o prqstat prqstat.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

debuglevel: $(O_FILES) debuglevel.cc
	$(CXX) $(CXXFLAGS) -o debuglevel debuglevel.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

addlink: $(O_FILES) addlink.cc
	$(CXX) $(CXXFLAGS) -o addlink addlink.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

dellink: $(O_FILES) dellink.cc
	$(CXX) $(CXXFLAGS) -o dellink dellink.cc $(O_FILES) $(TOOL_LIBS) $(LDFLAGS)

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

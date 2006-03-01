
SUB_DIRS = 

H_FILES = 

FILES = dummyproducer daqcat daqtail dummybinaryproducer binary2record \
raw2record dummyasciiproducer

include ../../../config.make

all: dummyproducer daqcat daqtail dummybinaryproducer binary2record \
     raw2record dummyasciiproducer

prog: bindir

include::

lib::

dummyproducer: dummyproducer.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ dummyproducer.cc $(APILIB) $(LDFLAGS) 

daqcat: daqcat.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ daqcat.cc $(APILIB) $(LDFLAGS) 

daqtail: daqtail.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ daqtail.cc $(APILIB) $(LDFLAGS) 

dummybinaryproducer: dummybinaryproducer.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ dummybinaryproducer.cc $(APILIB) $(LDFLAGS) 

dummyasciiproducer: dummyasciiproducer.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ dummyasciiproducer.cc $(APILIB) $(LDFLAGS) 

binary2record: binary2record.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ binary2record.cc $(APILIB) $(LDFLAGS) 

raw2record: raw2record.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ raw2record.cc $(APILIB) $(LDFLAGS) 

bindir: $(FILES)
	$(DIRINST) $(DAQBINDIR)
	$(BININST) $(FILES) $(DAQBINDIR)

install: prog
	$(DIRINST) $(INST_BINDIR)
	$(BININST) $(FILES) $(INST_BINDIR)

clean::
	rm -f *.o

realclean: clean
	rm -f $(FILES)
	rm -f .depend
	:> .depend

include .depend

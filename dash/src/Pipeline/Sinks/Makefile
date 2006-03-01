
SUB_DIRS = 

H_FILES = 

FILES = segmenter recorddump record2raw record2binary

include ../../../config.make

all: segmenter recorddump record2raw record2binary

prog: bindir

include::

lib::

segmenter: segmenter.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ segmenter.cc $(APILIB) $(LDFLAGS) 

recorddump: recorddump.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ recorddump.cc $(APILIB) $(LDFLAGS) 

record2raw: record2raw.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ record2raw.cc $(APILIB) $(LDFLAGS) 

record2binary: record2binary.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ record2binary.cc $(APILIB) $(LDFLAGS) 

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

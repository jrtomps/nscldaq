
SUB_DIRS = 

H_FILES = 

FILES = sieve sample ratemeter asciionramp binaryonramp daqtee 

include ../../../config.make

all: sieve sample ratemeter asciionramp binaryonramp daqtee

onramps: asciionramp binaryonramp

prog: bindir

include::

lib::

sieve: sieve.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ sieve.cc $(APILIB) $(LDFLAGS) 

sample: sample.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ sample.cc $(APILIB) $(LDFLAGS) 

ratemeter: ratemeter.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ ratemeter.cc $(APILIB) $(LDFLAGS) 

asciionramp: asciionramp.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ asciionramp.cc $(APILIB) $(LDFLAGS) 

binaryonramp: binaryonramp.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ binaryonramp.cc $(APILIB) $(LDFLAGS) 

daqtee: daqtee.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ daqtee.cc $(APILIB) $(LDFLAGS) 

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

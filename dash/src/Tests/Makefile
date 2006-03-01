
SUB_DIRS = 

H_FILES = 

FILES = bytebuffer record fdinputstream fdoutputstream bufferedrecordreader \
bufferedrecordwriter fdselector

CPPUNIT = -D CPPUNIT
LDDCPPUNIT = -lcppunit -ldl

include ../../config.make

all: bytebuffer record fdinputstream fdoutputstream bufferedrecordreader \
bufferedrecordwriter fdselector

include::

lib::

bytebuffer: bytebuffer.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ bytebuffer.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

record: record.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ record.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

fdinputstream: fdinputstream.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ fdinputstream.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

fdoutputstream: fdoutputstream.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ fdoutputstream.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

bufferedrecordreader: bufferedrecordreader.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ bufferedrecordreader.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

bufferedrecordwriter: bufferedrecordwriter.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ bufferedrecordwriter.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

fdselector: fdselector.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) $(CPPUNIT) -o $@ fdselector.cc $(APILIB) $(LDDCPPUNIT) $(LDFLAGS) 

install::

clean::
	rm -f *.o

realclean: clean
	rm -f $(FILES)
	rm -f .depend
	:> .depend

include .depend

SPECTRO_FLAGS=$(shell /usr/opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /usr/opt/spectrodaq/bin/spectrodaq-conf --libs)

# If you want to run the program in test mode, uncomment
# the second definition of TESTFLAGS below.
TESTFLAGS=
#TESTFLAGS=-DDAQ_TEST_MODE

FLAGS=-g -O2 $(SPECTRO_FLAGS) $(TESTFLAGS)
CFLAGS=-g -O2 $(SPECTRO_FLAGS) $(TESTFLAGS)  
CXXFLAGS=-g -O2 $(SPECTRO_FLAGS) $(TESTFLAGS)
LDFLAGS=$(SPECTRO_LDFLAGS)

CC=g++
CXX=g++

prog_files=daqbuffer bufferrecv buffersend

all: $(prog_files)

daqbuffer: daqbuffer.cc
	$(CXX) $(CXXFLAGS) -o $@ daqbuffer.cc $(LDFLAGS)

bufferrecv: bufferrecv.cc
	$(CXX) $(CXXFLAGS) -o $@ bufferrecv.cc $(LDFLAGS)

buffersend: buffersend.cc
	$(CXX) $(CXXFLAGS) -o $@ buffersend.cc $(LDFLAGS)

clean::
	rm -f *.o

realclean: clean
	rm -f $(prog_files)

#
#   Make tests of the modules in the Event dispatching system.
#
#


SPECTRO_FLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --libs)
OS=$(shell uname | sed s/-//)


# If you want to run the program in test mode, uncomment
# the second definition of TESTFLAGS below.
TESTFLAGS=
#TESTFLAGS=-DDAQ_TEST_MODE

LinuxDEF=-D_NEED_BOOL_T

LinuxMOTIFLIBDIR = -L/usr/X11/lib -I/usr/X11/include

FLAGS= -ggdb2 -g2 $(SPECTRO_FLAGS) $(TESTFLAGS) 
CFLAGS= -ggdb2 -g2 $(SPECTRO_FLAGS) $(TESTFLAGS) -I. -I..
CXXFLAGS= -ggdb2 -g2 $(SPECTRO_FLAGS) $(TESTFLAGS) -I. $($(OS)DEF) -I.. \
	$($(OS)MOTIFLIBDIR)
LDFLAGS=$(SPECTRO_LDFLAGS) -L.. -lEventFramework -ltk -ltcl -lm $($(OS)MOTIFLIBDIR) \
	-lXm -lXt -lX11


CC=c++
CXX=c++

TESTS	=	MutexTest XtTest TclTest TkTest RegistryTest SocketTest \
		BufferTest FdMonitorTest TimerTest LocationTest EventTest \
		CFileEventTest LocationEventTest BufferEventTest \
		TimerEventTest CTCPEventsTest ConfigTest

all: $(TESTS)

clean:
	rm $(TESTS)


MutexTest: MutexTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o MutexTest $< $(LDFLAGS)


XtTest: XtTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o XtTest $< $(LDFLAGS) $($(OS)MOTIFLIBDIR) -lXm -lXt

TclTest: TclTest.cpp TCLExtension.o ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TclTest $< TCLExtension.o $(LDFLAGS)

TkTest: TkTest.cpp TCLExtension.o ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TkTest $< TCLExtension.o $(LDFLAGS)

RegistryTest: RegistryTest.cpp CTestClasses.h ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o RegistryTest $< $(LDFLAGS)

SocketTest: SocketTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o SocketTest $< $(LDFLAGS)

TCLExtension.o: TCLExtension.cpp
	$(CXX) $(CXXFLAGS) -c $<

BufferTest: BufferTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o BufferTest $< $(LDFLAGS)

FdMonitorTest: FdMonitorTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o FdMonitorTest $< $(LDFLAGS)

TimerTest: TimerTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TimerTest $< $(LDFLAGS)

LocationTest: LocationTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o LocationTest $< $(LDFLAGS)


EventTest: EventTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o EventTest $< $(LDFLAGS)

CFileEventTest: CFileEventTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o CFileEventTest $< $(LDFLAGS)

LocationEventTest: LocationEventTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o LocationEventTest $< $(LDFLAGS)

BufferEventTest: BufferEventTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o BufferEventTest $< $(LDFLAGS)

TimerEventTest: TimerEventTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TimerEventTest $< $(LDFLAGS)

CTCPEventsTest: CTCPEventsTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o CTCPEventsTest $< $(LDFLAGS)

ConfigTest: ConfigTest.cpp ../libEventFramework.a
	$(CXX) $(CXXFLAGS) -o ConfigTest $< $(LDFLAGS)
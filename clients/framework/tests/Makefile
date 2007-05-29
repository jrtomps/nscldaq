#
#   Make tests of the modules in the Event dispatching system.
#
#


SPECTRO_FLAGS=$(shell /usr/opt/spectrodaq-lite/bin/spdaqlite-conf --cflags)
SPECTRO_LDFLAGS=$(shell /usr/opt/spectrodaq-lite/bin/spdaqlite-conf --libs)
OS=$(shell uname | sed s/-//)


# If you want to run the program in test mode, uncomment
# the second definition of TESTFLAGS below.
TESTFLAGS=
#TESTFLAGS=-DDAQ_TEST_MODE

LinuxDEF=-D_NEED_BOOL_T

LinuxMOTIFLIBDIR = -L/usr/X11/lib -I/usr/X11/include

FLAGS= -ggdb2 -g2 $(SPECTRO_FLAGS) $(TESTFLAGS) 
CFLAGS= -ggdb2 -g2 $(SPECTRO_FLAGS) $(TESTFLAGS) -I. -I.. -I../../SpectrodaqLite
CXXFLAGS= -ggdb2 -g2 $(SPECTRO_FLAGS) $(TESTFLAGS) -I. $($(OS)DEF) -I.. \
	$($(OS)MOTIFLIBDIR) -I../../SpectrodaqLite
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


MutexTest: MutexTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o MutexTest $< $(LDFLAGS)


XtTest: XtTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o XtTest $< $(LDFLAGS) $($(OS)MOTIFLIBDIR) -lXm -lXt

TclTest: TclTest.cpp TCLExtension.o ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TclTest $< TCLExtension.o $(LDFLAGS)

TkTest: TkTest.cpp TCLExtension.o ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TkTest $< TCLExtension.o $(LDFLAGS)

RegistryTest: RegistryTest.cpp CTestClasses.h ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o RegistryTest $< $(LDFLAGS)

SocketTest: SocketTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o SocketTest $< $(LDFLAGS)

TCLExtension.o: TCLExtension.cpp
	$(CXX) $(CXXFLAGS) -c $<

BufferTest: BufferTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o BufferTest $< $(LDFLAGS)

FdMonitorTest: FdMonitorTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o FdMonitorTest $< $(LDFLAGS)

TimerTest: TimerTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TimerTest $< $(LDFLAGS)

LocationTest: LocationTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o LocationTest $< $(LDFLAGS)


EventTest: EventTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o EventTest $< $(LDFLAGS)

CFileEventTest: CFileEventTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o CFileEventTest $< $(LDFLAGS)

LocationEventTest: LocationEventTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o LocationEventTest $< $(LDFLAGS)

BufferEventTest: BufferEventTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o BufferEventTest $< $(LDFLAGS)

TimerEventTest: TimerEventTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o TimerEventTest $< $(LDFLAGS)

CTCPEventsTest: CTCPEventsTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o CTCPEventsTest $< $(LDFLAGS)

ConfigTest: ConfigTest.cpp ../.libs/libEventFramework.a
	$(CXX) $(CXXFLAGS) -o ConfigTest $< $(LDFLAGS)
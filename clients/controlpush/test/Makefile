OBJECTS=v890.o TestFixture.o

CXX=g++-2.95
CXXFLAGS= -c -ggdb -I.. 

EPICS_ROOT=/opt/epics
EPICS_BIN=$(EPICS_ROOT)/bin/linux-x86
EPICS_LIB=$(EPICS_ROOT)/lib/linux-x86
EPICS_INCLUDES=-I/opt/epics/include -I/opt/epics/include/os/Linux \
	       -I../../Exception -I../Exception -I../Headers \
		-I../../Headers 
EPICS_LDFLAGS=-L$(EPICS_LIB) -lca -Wl,"-rpath=$(EPICS_LIB)" 


LDFLAGS=$(EPICS_LDFLAGS) -L/usr/opt/daq/lib -lException

.cpp.o:
	$(CXX) $(CXXFLAGS) $(EPICS_INCLUDES) $<


TESTS=     ChannelListTests.o ChannelTest.o VisitorTests.o ApplicationTests.o

TESTTARGETS= ../CChannel.o ../CAllOk.o ../CSGBuilder.o ../CSingleUpdater.o  \
	../CLookupVisitor.o ../CBuildChannelData.o ../CChannelList.o \
	../CNoUnitsException.o  ../CApplication.o ../CSocket.o ../CStrings.o \
	../CTCPBadSocketState.o ../CApplicationSerializer.o ../CTCPNoSuchHost.o \
	../CTCPConnectionFailed.o ../CTCPConnectionLost.o ../CTCPNoSuchService.o

tests: TestRunner.o $(TESTS) $(TESTTARGETS)
	$(CXX) -o tests TestRunner.o $(TESTS) $(TESTTARGETS) \
	$(LDFLAGS) -lcppunit 

depend:
	makedepend *.cpp


clean:
	rm -f *.o tests


test: tests
	./tests
OBJECTS=v890.o TestFixture.o

CXX=g++-2.95 -D_GNU_SOURCE
CXXFLAGS= -c -ggdb -I.. -I../../Headers  -I../../Exception


LDFLAGS= -L/usr/opt/daq/lib -lException -lpub

.cpp.o:
	$(CXX) $(CXXFLAGS)  $<


TESTS=     CFileSinkTests.o SinkCreatorTests.o FactoryTests.o grep.o \
	LocalMonitoredProgram.o

TESTTARGETS= ../CSink.o ../CFileSink.o ../CFileSinkCreator.o \
	../CSinkFactory.o \
	../CMonitoredProgram.o ../CLocalMonitoredProgram.o

tests: TestRunner.o $(TESTS) $(TESTTARGETS)
	$(CXX) -o tests TestRunner.o $(TESTS) $(TESTTARGETS) \
	$(LDFLAGS) -lcppunit 

depend:
	makedepend *.cpp


clean:
	rm -f *.o tests


test: tests
	./tests

CXX=g++-2.95
CXXFLAGS= -c -ggdb -I. -I/usr/opt/daq/include -D_DEBUG -DDESIGN_BY_CONTRACT \
	-DCHECK_ALL -pedantic -I.. -I../../Headers


.cpp.o:
	$(CXX) $(CXXFLAGS) $<


TESTS=     DataTests.o ConstructTests.o TerminationTests.o \
	TriggerTimeTagTests.o geotests.o RegisterTests.o \
	AcqModeTests.o MatchTiming.o ResEdgeTests.o ReadoutModes.o \
	AdjustTests.o MiscTests.o

RDTESTS=  ReadTests.o

TESTTARGETS= ../CCAENV1x90.o

tests:  functests rdtests


#    Individual function tests...

functests: TestRunner.o $(TESTS) $(TESTTARGETS)
	$(CXX) -o functests TestRunner.o $(TESTS) $(TESTTARGETS) -lcppunit \
	-L/usr/opt/daq/lib -lDeviceSupport -lVme -lVmeAPI -lbtp -lpthread

#  Tests which try to read the module (separated since functests takes about
#  1/2hr to run on a 1290N (dont' want to think about an 1190A since time
#  should be proportional to the channel count.

rdtests: TestRunner.o $(RDTESTS) $(TESTTARGETS)
	$(CXX) -o rdtests TestRunner.o $(RDTESTS) $(TESTTARGETS) -lcppunit \
	-L/usr/opt/daq/lib -lDeviceSupport -lVme -lVmeAPI -lbtp -lpthread

depend:
	makedepend *.cpp


clean:
	rm -f *.o functests rdtests



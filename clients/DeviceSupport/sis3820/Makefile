OBJECTS=v890.o TestFixture.o

CXX=g++
CXXFLAGS= -c -ggdb -I/usr/opt/daq/include

MODULEFLAGS= -c -ggdb -I/usr/opt/daq/include

.cpp.o:
	$(CXX) $(CXXFLAGS) $<


TESTS=sis3820tests.o                               # List sources here.

MODULES=CSIS3820.o
MODULELIBS=-L../.libs -L/usr/opt/daq/lib -lDeviceSupport -lVmeAPI -lException \
	-lbtp -pthread


tests: TestRunner.o $(TESTS) $(MODULES)
	g++ -o tests TestRunner.o $(TESTS) $(MODULES) $(MODULELIBS) -lcppunit


CSIS3820.o: CSIS3820.cpp
	$(CXX) $(MODULEFLAGS) CSIS3820.cpp

depend:
	makedepend *.cpp


clean:
	rm -f *.o tests

test: tests
	./tests

docs:  maindocs.oxy modes.oxy readout.oxy setup.oxy
	doxygen

# DO NOT DELETE

CSIS3820.o: CSIS3820.h
TestRunner.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
TestRunner.o: /usr/include/cppunit/Portability.h
TestRunner.o: /usr/include/cppunit/config-auto.h
TestRunner.o: /usr/include/cppunit/extensions/TestFactory.h
TestRunner.o: /usr/include/cppunit/ui/text/TestRunner.h
TestSuite.o: /usr/include/cppunit/extensions/HelperMacros.h
TestSuite.o: /usr/include/cppunit/Portability.h
TestSuite.o: /usr/include/cppunit/config-auto.h
TestSuite.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
TestSuite.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
TestSuite.o: /usr/include/cppunit/extensions/TestFactory.h
TestSuite.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
TestSuite.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
TestSuite.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
TestSuite.o: /usr/include/cppunit/TestCaller.h
TestSuite.o: /usr/include/cppunit/Exception.h
TestSuite.o: /usr/include/cppunit/SourceLine.h
TestSuite.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
TestSuite.o: /usr/include/cppunit/TestCase.h
TestSuite.o: /usr/include/cppunit/TestAssert.h
TestSuite.o: /usr/include/cppunit/Asserter.h
TestSuite.o: /usr/include/cppunit/TestFixture.h Asserts.h
sis3820tests.o: /usr/include/cppunit/extensions/HelperMacros.h
sis3820tests.o: /usr/include/cppunit/Portability.h
sis3820tests.o: /usr/include/cppunit/config-auto.h
sis3820tests.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
sis3820tests.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
sis3820tests.o: /usr/include/cppunit/extensions/TestFactory.h
sis3820tests.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
sis3820tests.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
sis3820tests.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
sis3820tests.o: /usr/include/cppunit/TestCaller.h
sis3820tests.o: /usr/include/cppunit/Exception.h
sis3820tests.o: /usr/include/cppunit/SourceLine.h
sis3820tests.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
sis3820tests.o: /usr/include/cppunit/TestCase.h
sis3820tests.o: /usr/include/cppunit/TestAssert.h
sis3820tests.o: /usr/include/cppunit/Asserter.h
sis3820tests.o: /usr/include/cppunit/TestFixture.h Asserts.h CSIS3820.h

CXX=g++
CXXFLAGS= -c -ggdb -I/usr/opt/daq/include


.cpp.o:
	$(CXX) $(CXXFLAGS) $<


TESTS=  sis3600tests.o CSIS3600.o                              # List sources here.

tests: TestRunner.o $(TESTS)
	g++ -L/usr/opt/daq/lib -o tests TestRunner.o $(TESTS) \
	-lDeviceSupport  -lVmeAPI -lException  -lbtp -lpthread -lcppunit

depend:
	makedepend *.cpp


clean:
	rm -f *.o tests


test: tests
	./tests# DO NOT DELETE

docs:
	doxygen

CSIS3600.o: CSIS3600.h
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
sis3600tests.o: /usr/include/cppunit/extensions/HelperMacros.h
sis3600tests.o: /usr/include/cppunit/Portability.h
sis3600tests.o: /usr/include/cppunit/config-auto.h
sis3600tests.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
sis3600tests.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
sis3600tests.o: /usr/include/cppunit/extensions/TestFactory.h
sis3600tests.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
sis3600tests.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
sis3600tests.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
sis3600tests.o: /usr/include/cppunit/TestCaller.h
sis3600tests.o: /usr/include/cppunit/Exception.h
sis3600tests.o: /usr/include/cppunit/SourceLine.h
sis3600tests.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
sis3600tests.o: /usr/include/cppunit/TestCase.h
sis3600tests.o: /usr/include/cppunit/TestAssert.h
sis3600tests.o: /usr/include/cppunit/Asserter.h
sis3600tests.o: /usr/include/cppunit/TestFixture.h Asserts.h CSIS3600.h

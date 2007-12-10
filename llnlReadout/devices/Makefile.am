lib_LTLIBRARIES 		= libVMUSBDevices.la
libVMUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp 	\
					C3820.cpp		\
					C785.cpp		\
					CCAENChain.cpp		\
					CStack.cpp		\
					C830.cpp		\
					CMarker.cpp


INCLUDES	= 	-I$(top_builddir)/vmusb \
			-I$(top_builddir)/daqconfig \
			@TCL_FLAGS@ $(SPECTRODAQ_FLAGS)


libVMUSBDevices_la_LIBADD	= $(top_builddir)/vmusb/libVMUSB.la \
				  @TCL_LDFLAGS@ $(SPECTRODAQ_LDFLAGS)

include_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					C3820.h			\
					C785.h			\
					CCAENChain.h		\
					CStack.h		\
					C830.h			\
					CMarker.h


TESTS=configtest.cpp c785test.cpp c3820test.cpp

tests: TestRunner.cpp $(TESTS)
	$(CXX) -g -o tests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	TestRunner.cpp  $(TESTS) -L$(prefix)/lib -lVMUSBDevices -lVMUSB \
	-Wl,"-rpath=$(prefix)/lib"   \
	$(CPPUNIT_LDFLAGS)

installcheck-local:	tests
	./tests

EXTRA_DIST=TestRunner.cpp $(TESTS)
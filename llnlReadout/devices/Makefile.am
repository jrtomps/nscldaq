lib_LTLIBRARIES 		= libVMUSBDevices.la
libVMUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp 	\
					C3820.cpp		\
					C785.cpp		\
					CCAENChain.cpp		\
					CStack.cpp		\
					C830.cpp		\
					CMarker.cpp		\
					C3804.cpp		\
					CNADC2530.cpp		\
					CMADC32.cpp		\
					CMADCScaler.cpp		\
					CV1x90.cpp		\
					CV977.cpp		\
					CXLM.cpp		\
					CMASE.cpp		\
					CMADCChain.cpp	\
					CHINP.cpp \
					CPSD.cpp	\
					CV1729.cpp


INCLUDES	= 	-I$(top_builddir)/vmusb 	\
			-I$(top_builddir)/daqconfig 	\
			-I$(daqdir)/include		\
			@TCL_FLAGS@ $(SPECTRODAQ_FLAGS)


libVMUSBDevices_la_LIBADD	= $(top_builddir)/vmusb/libVMUSB.la \
				  @TCL_LDFLAGS@ $(SPECTRODAQ_LDFLAGS) \
				-lm

include_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					C3820.h			\
					C785.h			\
					CCAENChain.h		\
					CStack.h		\
					C830.h			\
					CMarker.h		\
					C3804.h			\
					CNADC2530.h		\
					CMADC32.h		\
					CMADCScaler.h		\
					CV1x90.h		\
					CV977.h			\
					CXLM.h			\
					CMASE.h			\
					CMADCChain.h		\
					MADC32Registers.h	\
					CHINP.h \
					CPSD.h			\
					CV1729.h


TESTS=configtest.cpp c785test.cpp c3820test.cpp

tests: TestRunner.cpp $(TESTS)
	$(CXX) -g -o tests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	TestRunner.cpp  $(TESTS) -L$(prefix)/lib -lVMUSBDevices -lVMUSB -lConfig \
	$(NSCLDAQ_LIB) -ltclPlus -lException  $(TCL_LDFLAGS) \
	-Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" \
	$(CPPUNIT_LDFLAGS)

configtests:	TestRunner.cpp configtest.cpp
	$(CXX) -g -o configtests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	TestRunner.cpp  configtest.cpp -L$(prefix)/lib -lVMUSBDevices -lVMUSB -lConfig \
	$(NSCLDAQ_LIB) -ltclPlus -lException  $(TCL_LDFLAGS) \
	-Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" \
	$(CPPUNIT_LDFLAGS)


installcheck-local:	tests
	./tests

EXTRA_DIST=TestRunner.cpp $(TESTS)
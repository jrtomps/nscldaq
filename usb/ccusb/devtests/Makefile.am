noinst_PROGRAMS = unittests


TESTS = cccusbtest.cpp

unittests_SOURCES = TestRunner.cpp $(TESTS) 

unittests_LDADD = @top_builddir@/usb/ccusb/devices/libCCUSBDevices.la \
									@top_builddir@/usb/ccusb/ccusb/libCCUSB.la \
									@top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la \
									$(NSCLDAQ_LIB) @CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@


unittests_CPPFLAGS = @TCL_CPPFLAGS@ $(CPPUNIT_INCLUDES) $(THREADCXX_FLAGS) $(AM_CXXFLAGS) \
											-I@srcdir@/../. -I@srcdir@/../devices/. \
											-I@srcdir@/../ccusb/. -I@prefix@/include

unittests_LDFLAGS = @LIBTCLPLUS_LDFLAGS@  $(TCL_LDFLAGS) \
                  -Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" 

check-TESTS:
	./unittests 

EXTRA_DIST=TestRunner.cpp $(TESTS) Asserts.h

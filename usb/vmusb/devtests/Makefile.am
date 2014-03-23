noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp cvmusbtest.cpp

unittests_DEPENDENCIES = @top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
									@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
									@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la \
									$(NSCLDAQ_LIB) 

unittests_LDADD = @top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
									@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
									@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la \
									$(NSCLDAQ_LIB) @CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

unittests_CXXFLAGS = $(CPPUNIT_INCLUDES) $(THREADCXX_FLAGS) $(AM_CXXFLAGS) \
											-I@srcdir@/../. -I@srcdir@/../devices/. \
											-I@srcdir@/../vmusb/.

unittests_LDFLAGS = @LIBTCLPLUS_LDFLAGS@  $(TCL_LDFLAGS) \
                  -Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" 

check-TESTS:
	./unittests 

EXTRA_DIST=TestRunner.cpp $(TESTS)

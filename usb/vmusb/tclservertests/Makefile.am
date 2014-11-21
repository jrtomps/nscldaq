UNITTEST_MODULES =	@srcdir@/TestRunner.cpp \
										@srcdir@/mxdcrcbustest.cpp 

noinst_PROGRAMS = unittests

unittests_SOURCES = @srcdir@/TestRunner.cpp \
										@srcdir@/mxdcrcbustest.cpp 
unittests_LDADD = @top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
										@top_builddir@/usb/vmusb/tclserver/libVMUSBControlServer.la \
										@LIBTCLPLUS_LDFLAGS@ \
										@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la \
										$(CPPUNIT_LDFLAGS)
unittests_CPPFLAGS = -I@top_srcdir@/usb/vmusb/vmusb  \
										 $(CPPUNIT_FLAGS) \
										 -I@top_srcdir@/usb/vmusb/tclserver \
										 -I@top_srcdir@/usb/vmusb/devices


check-TESTS : $(UNITTEST_MODULES)
		./unittests


EXTRA_DIST = FakeGlobals.h

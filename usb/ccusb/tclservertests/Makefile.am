noinst_PROGRAMS = unittests

unittests_SOURCES = @srcdir@/TestRunner.cpp 
unittests_LDADD = @top_builddir@/usb/ccusb/ccusb/libCCUSB.la \
									@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
										@top_builddir@/usb/ccusb/tclserver/libCCUSBControlServer.la \
										@LIBTCLPLUS_LDFLAGS@ \
										@top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la \
										$(CPPUNIT_LDFLAGS) \
										@TCL_LDFLAGS@ \
										@top_builddir@/base/thread/libdaqthreads.la 
unittests_CPPFLAGS = -I@top_srcdir@/usb/ccusb/ccusb  \
										 -I@top_srcdir@/usb/common/configurableobject \
										 $(CPPUNIT_FLAGS) \
										 -I@top_srcdir@/usb/ccusb/tclserver \
										 -I@top_srcdir@/usb/ccusb/devices \
										 -I@top_srcdir@/base/thread \
										 @LIBTCLPLUS_CFLAGS@ \
										 @TCL_FLAGS@


check-TESTS : $(UNITTEST_MODULES)
		./unittests


EXTRA_DIST = FakeGlobals.h

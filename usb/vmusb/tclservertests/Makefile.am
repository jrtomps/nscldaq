noinst_PROGRAMS = unittests

unittests_SOURCES = @srcdir@/TestRunner.cpp \
										@srcdir@/mxdcrcbustest.cpp   \
										@srcdir@/modulecommandtest.cpp
#										@srcdir@/v812test.cpp
unittests_LDADD = @top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
									@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
										@top_builddir@/usb/vmusb/tclserver/libVMUSBControlServer.la \
										@LIBTCLPLUS_LDFLAGS@ \
										@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la \
										$(CPPUNIT_LDFLAGS) \
										@TCL_LDFLAGS@ \
										@top_builddir@/base/thread/libdaqthreads.la 
unittests_CPPFLAGS = -I@top_srcdir@/usb/vmusb/vmusb  \
										 -I@top_srcdir@/usb/common/configurableobject \
										 $(CPPUNIT_FLAGS) \
										 -I@top_srcdir@/usb/vmusb/tclserver \
										 -I@top_srcdir@/usb/vmusb/devices \
										 @LIBTCLPLUS_CFLAGS@ \
										 @TCL_FLAGS@


check-TESTS : $(UNITTEST_MODULES)
		./unittests


EXTRA_DIST = FakeGlobals.h

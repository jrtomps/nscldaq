noinst_PROGRAMS = unittests

unittests_SOURCES = @srcdir@/TestRunner.cpp  @srcdir@/modulecommandtest.cpp \
										@srcdir@/c894test.cpp @srcdir@/cph7106test.cpp \
										@srcdir@/cccusbcontroltest.cpp\
										@srcdir@/ctclmoduletest.cpp

unittests_LDADD = @top_builddir@/usb/ccusb/ccusb/libCCUSB.la \
									@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
										@top_builddir@/usb/ccusb/tclserver/libCCUSBControlServer.la \
										@top_builddir@/usb/ccusb/rdothread/libCCUSBRdoThread.la \
										@top_builddir@/usb/ccusb/router/libCCUSBRouter.la \
										@LIBTCLPLUS_LDFLAGS@ \
										@top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la \
										$(CPPUNIT_LDFLAGS) \
										@TCL_LDFLAGS@ \
										@top_builddir@/base/thread/libdaqthreads.la 
unittests_CPPFLAGS = -I@top_srcdir@/usb/ccusb/ccusb  \
										 -I@top_srcdir@/usb/common/configurableobject \
										 -I@top_srcdir@/usb/common/slowcontrols \
										 -I@top_srcdir@/usb/ccusb/router \
										 -I@top_srcdir@/usb/ccusb/rdothread \
										 $(CPPUNIT_FLAGS) \
										 -I@top_srcdir@/usb/ccusb/tclserver \
										 -I@top_srcdir@/usb/ccusb/devices \
										 -I@top_srcdir@/base/thread \
										 @LIBTCLPLUS_CFLAGS@ \
										 @TCL_FLAGS@


check-TESTS : $(UNITTEST_MODULES)
		./unittests


EXTRA_DIST = FakeGlobals.h

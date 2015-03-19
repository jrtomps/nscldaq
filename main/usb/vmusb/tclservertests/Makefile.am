noinst_PROGRAMS = unittests

unittests_SOURCES = @srcdir@/TestRunner.cpp \
		@srcdir@/mxdcrcbustest.cpp   \
		@srcdir@/modulecommandtest.cpp \
		@srcdir@/cgdgtest.cpp  \
		@srcdir@/cv6533test.cpp  \
		@srcdir@/cchicotriggertest.cpp \
		@srcdir@/ctclcontrolmoduletest.cpp \
		@srcdir@/cxlmcontrolstest.cpp \
		@srcdir@/cvmusbmoduletest.cpp \
		@srcdir@/cwienermdgg16test.cpp \
		@srcdir@/cmdgg16controltest.cpp


unittests_LDADD = @top_builddir@/usb/vmusb/tclserver/libVMUSBControlServer.la \
		@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
		@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
		@LIBTCLPLUS_LDFLAGS@ \
		@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la \
		@top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
		@top_builddir@/usb/vmusb/rdothread/libVMUSBRdoThread.la \
		@top_builddir@/usb/vmusb/router/libVMUSBRouter.la \
		$(CPPUNIT_LDFLAGS) \
		@TCL_LDFLAGS@ \
		@top_builddir@/base/thread/libdaqthreads.la 

unittests_CPPFLAGS = -I@top_srcdir@/usb/vmusb/vmusb  \
		 -I@top_srcdir@/usb/common/configurableobject \
		 -I@top_srcdir@/usb/common/slowcontrols \
		 $(CPPUNIT_FLAGS) \
		 -I@top_srcdir@/usb/vmusb/tclserver \
		 -I@top_srcdir@/usb/vmusb/devices \
		 -I@top_srcdir@/usb/vmusb/router \
		 -I@top_srcdir@/usb/vmusb/rdothread \
		 -I@top_srcdir@/base/thread \
		 @LIBTCLPLUS_CFLAGS@ \
		 @TCL_FLAGS@


check-TESTS : $(UNITTEST_MODULES)
		./unittests


EXTRA_DIST = FakeGlobals.h

noinst_PROGRAMS = unittests


TEST_FILES = cccusbtest.cpp

unittests_SOURCES = TestRunner.cpp $(TEST_FILES) 

unittests_LDADD =  \
		@top_builddir@/usb/ccusb/ccusb/libCCUSB.la            \
		@top_builddir@/usb/ccusb/daqconfig/libCCUSBDaqConfig.la  \
		@top_builddir@/base/thread/libdaqthreads.la		\
		@top_builddir@/base/tcpip/libTcp.la                   \
		@LIBTCLPLUS_LDFLAGS@                                  \
		@LIBEXCEPTION_LDFLAGS@                                \
		@TCL_LDFLAGS@				              \
		@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@


unittests_CPPFLAGS = \
		@TCL_CPPFLAGS@ \
		$(CPPUNIT_INCLUDES) \
		$(THREADCXX_FLAGS) \
		$(AM_CXXFLAGS) \
		-I@top_srcdir@/usb/ccusb \
		-I@top_srcdir@/usb/ccusb/daqconfig \
		-I@top_srcdir@/usb/ccusb/ccusb \
		-I@top_srcdir@/usb/ccusb/core	\
		-I@prefix@/include

unittests_LDFLAGS =  -Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" 

TESTS = @builddir@/unittests 

EXTRA_DIST=TestRunner.cpp $(TESTS) Asserts.h

lib_LTLIBRARIES		= 	libCCUSBRouter.la
libCCUSBRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

noinst_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

libCCUSBRouter_la_CPPFLAGS		= -I@top_srcdir@/usb/ccusb		\
			@TCL_CPPFLAGS@				\
			@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/headers		\
			-I@top_srcdir@/base/dataflow		\
			-I@top_srcdir@/daq/format		\
                        -I@top_srcdir@/daq/eventbuilder  \
			-I@top_srcdir@/base/thread @THREADCXX_FLAGS@ \
			-I@top_srcdir@/usb/ccusb/devices



libCCUSBRouter_la_LIBADD	= @top_builddir@/usb/ccusb/devices/libCCUSBDevices.la \
			@LIBTCLPLUS_LDFLAGS@				\
			@top_builddir@/base/dataflow/libRingBuffer.la \
			@top_builddir@/daq/format/libdataformat.la		\
			@top_builddir@/base/thread/libdaqthreads.la @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes

lib_LTLIBRARIES		= 	libCCUSBRouter.la
libCCUSBRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

noinst_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

INCLUDES		= -I@top_srcdir@/usb/ccusb/threadcomm  \
			-I@top_srcdir@/usb/ccusb		\
			@LIBTCLPLUS_CFLAGS@			\
			-I@top_srcdir@/base/headers		\
			-I@top_srcdir@/base/dataflow		\
			-I@top_srcdir@/daq/format		\
			-I@top_srcdir@/base/thread @THREADCXX_FLAGS@



libCCUSBRouter_la_LIBADD	= @top_srcdir@/usb/ccusb/threadcomm/libCCUSBThreadComm.la \
			@LIBTCLPLUS_LDFLAGS@		\
			@top_builddir@/base/dataflow/libRingBuffer.la \
			@top_builddir@/daq/format/libdataformat.la		\
			@top_builddir@/base/thread/libdaqthreads.la @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes

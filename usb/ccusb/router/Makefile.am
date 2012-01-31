lib_LTLIBRARIES		= 	libRouter.la
libRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

noinst_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

INCLUDES		= -I@top_srcdir@/usb/ccusb/threadcomm  \
			-I@top_srcdir@/usb/ccusb		\
			-I@top_srcdir@/base/exception		\
			-I@top_srcdir@/base/headers		\
			-I@top_srcdir@/base/dataflow		\
			-I@top_srcdir@/daq/format		\
			-I@top_srcdir@/base/thread



libRouter_la_LIBADD	= @top_srcdir@/usb/ccusb/threadcomm/libThreadComm.la \
			@top_builddir@/base/exception/libException.la \
			@top_builddir@/base/dataflow/libRingBuffer.la \
			@top_builddir@/daq/format/libdataformat.la		\
			@top_builddir@/base/thread/libdaqthreads.la
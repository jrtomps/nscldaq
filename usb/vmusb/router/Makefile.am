lib_LTLIBRARIES		= 	libVMUSBRouter.la
libVMUSBRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

noinst_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

libVMUSBRouter_la_CPPFLAGS		=  -DLAST_CHANCE \
			-I@top_srcdir@/usb/vmusb/tclserver $(TCL_FLAGS) \
			-I@top_srcdir@/usb/vmusb/vmusb -I@top_srcdir@/base/thread \
			@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/headers  \
			-I@top_srcdir@/usb/vmusb	\
			-I@top_srcdir@/base/dataflow	\
			-I@top_srcdir@/daq/format   \
                        -I@top_srcdir@/daq/eventbuilder

libVMUSBRouter_la_CXXFLAGS=@THREADCXX_FLAGS@

libVMUSBRouter_la_LIBADD	= @LIBTCLPLUS_LDFLAGS@	\
	@top_builddir@/base/thread/libdaqthreads.la \
	@top_builddir@/base/dataflow/libRingBuffer.la \
	@top_builddir@/daq/format/libdataformat.la @THREADLD_FLAGS@
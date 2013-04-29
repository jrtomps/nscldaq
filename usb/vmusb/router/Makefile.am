lib_LTLIBRARIES		= 	libVMUSBRouter.la
libVMUSBRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

noinst_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

INCLUDES		= -I@top_srcdir@/usb/vmusb/threadcomm -DLAST_CHANCE \
			-I@top_srcdir@/usb/vmusb/tclserver $(TCL_FLAGS) \
			-I@top_srcdir@/usb/vmusb/vmusb -I@top_srcdir@/base/thread \
			-I@top_srcdir@/base/exception \
			-I@top_srcdir@/base/headers  \
			-I@top_srcdir@/usb/vmusb	\
			-I@top_srcdir@/base/dataflow	\
			-I@top_srcdir@/daq/format   \
                        -I@top_srcdir@/daq/eventbuilder

libVMUSBRouter_la_CXXFLAGS=@THREADCXX_FLAGS@

libVMUSBRouter_la_LIBADD	= @top_srcdir@/base/exception/libException.la \
	@top_srcdir@/usb/vmusb/threadcomm/libVMUSBThreadComm.la \
	@top_srcdir@/base/thread/libdaqthreads.la \
	@top_srcdir@/base/dataflow/libRingBuffer.la \
	@top_srcdir@/daq/format/libdataformat.la @THREADLD_FLAGS@
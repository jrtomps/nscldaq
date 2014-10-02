lib_LTLIBRARIES		= libVMUSBCommands.la
libVMUSBCommands_la_SOURCES	=	CBeginRun.cpp	\
				CEndRun.cpp	\
				CPauseRun.cpp	\
				CResumeRun.cpp	\
				CExit.cpp	\
				tclUtil.cpp	

noinst_HEADERS		= 	CBeginRun.h	\
				CEndRun.h	\
				CPauseRun.h	\
				CResumeRun.h	\
				CExit.h		\
				tclUtil.h

libVMUSBCommands_la_CXXFLAGS=@THREADCXX_FLAGS@


INCLUDES		= 	-I@top_srcdir@/usb/vmusb/rdothread	\
				-I@top_srcdir@/usb/vmusb/router		\
				-I@top_srcdir@/usb/vmusb/daqconfig	\
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/usb/vmusb/vmusb		\
				-I@top_srcdir@/usb/vmusb/tclserver	\
				@LIBTCLPLUS_CFLAGS@	                \
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/thread		\
				$(TCL_FLAGS) 


libVMUSBCommands_la_LIBADD	= 	@top_builddir@/usb/vmusb/rdothread/libVMUSBRdoThread.la	\
				@top_builddir@/usb/vmusb/router/libVMUSBRouter.la	\
				@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la	\
				@LIBTCLPLUS_LDFLAGS@		\
				@top_builddir@/base/thread/libdaqthreads.la	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@
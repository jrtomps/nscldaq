lib_LTLIBRARIES		= libVMUSBCommands.la
libVMUSBCommands_la_SOURCES	=	CBeginRun.cpp	\
				CEndRun.cpp	\
				CPauseRun.cpp	\
				CResumeRun.cpp	\
				tclUtil.cpp	

noinst_HEADERS		= 	CBeginRun.h	\
				CEndRun.h	\
				CPauseRun.h	\
				CResumeRun.h	\
				tclUtil.h

libVMUSBCommands_la_CXXFLAGS=@THREADCXX_FLAGS@


INCLUDES		= 	-I@top_srcdir@/usb/vmusb/threadcomm	\
			  	-I@top_srcdir@/usb/vmusb/rdothread	\
				-I@top_srcdir@/usb/vmusb/router		\
				-I@top_srcdir@/usb/vmusb/daqconfig	\
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/usb/vmusb/vmusb		\
				@LIBTCLPLUS_CFLAGS@	\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/base/thread		\
				$(TCL_FLAGS) 


libVMUSBCommands_la_LIBADD	= 	@top_srcdir@/usb/vmusb/rdothread/libVMUSBRdoThread.la	\
				@top_srcdir@/usb/vmusb/threadcomm/libVMUSBThreadComm.la	\
				@top_srcdir@/usb/vmusb/router/libVMUSBRouter.la	\
				@top_srcdir@/usb/vmusb/daqconfig/libVMUSBConfig.la	\
				@LIBTCLPLUS_LDFLAGS@		\
				@top_srcdir@/base/exception/libException.la	\
				@top_srcdir@/base/thread/libdaqthreads.la	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@
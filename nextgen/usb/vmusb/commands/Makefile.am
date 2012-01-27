lib_LTLIBRARIES		= libCommands.la
libCommands_la_SOURCES	=	CBeginRun.cpp	\
				CEndRun.cpp	\
				CPauseRun.cpp	\
				CResumeRun.cpp	\
				tclUtil.cpp	

include_HEADERS		= 	CBeginRun.h	\
				CEndRun.h	\
				CPauseRun.h	\
				CResumeRun.h	\
				tclUtil.h

libCommands_la_CXXFLAGS=@THREADCXX_FLAGS@


INCLUDES		= 	-I@top_srcdir@/usb/vmusb/threadcomm	\
			  	-I@top_srcdir@/usb/vmusb/rdothread	\
				-I@top_srcdir@/usb/vmusb/router		\
				-I@top_srcdir@/usb/vmusb/daqconfig	\
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/base/tclplus		\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/base/thread		\
				$(TCL_FLAGS) 


libCommands_la_LIBADD	= 	@top_srcdir@/usb/vmusb/rdothread/libRdoThread.la	\
				@top_srcdir@/usb/vmusb/threadcomm/libThreadComm.la	\
				@top_srcdir@/usb/vmusb/router/libRouter.la	\
				@top_srcdir@/usb/vmusb/daqconfig/libConfig.la	\
				@top_srcdir@/base/tclplus/libtclPlus.la	\
				@top_srcdir@/base/exception/libException.la	\
				@top_srcdir@/base/thread/libdaqthreads.la	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@
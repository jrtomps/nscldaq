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

INCLUDES		= 	-I@top_srcdir@/usb/ccusb/threadcomm	\
			  	-I@top_srcdir@/usb/ccusb/rdothread	\
				-I@top_srcdir@/usb/ccusb/router		\
				-I@top_srcdir@/usb/ccusb/daqconfig	\
				-I@top_srcdir@/usb/ccusb		\
				-I@top_srcdir@/base/tclplus		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/thread		\
				$(TCL_FLAGS)

libCommands_la_LIBADD	= 	@top_builddir@/usb/ccusb/rdothread/libRdoThread.la	\
				@top_builddir@/usb/ccusb/threadcomm/libThreadComm.la	\
				@top_builddir@/usb/ccusb/router/libRouter.la	\
				@top_builddir@/usb/ccusb/daqconfig/libConfig.la	\
				@top_builddir@/base/tclplus/libtclPlus.la 	\
				@top_builddir@/base/thread/libdaqthreads.la	\
			        @top_builddir@/base/exception/libException.la \
				$(TCL_LDFLAGS)
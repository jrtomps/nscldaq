lib_LTLIBRARIES			= libControlServer.la
libControlServer_la_SOURCES	= 	CModuleCommand.cpp	\
					TclServer.cpp		\
					CControlModule.cpp	\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					Server.cpp

include_HEADERS			=	CSetCommand.h		\
					server.h		\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					TclServer.h

INCLUDES			=	-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/usb/tclcommon	\
					-I@top_srcdir@/usb/threadcom	\
					-I@top_srcdir@/base/tclplus	\
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/base/thread	\
					@TCL_FLAGS@



libControlServer_la_LIBADD	= @top_srcdir@/usb/tclcommon/libusbtclcommon.la	\
				@top_srcdir@/usb/threadcom/libThreadComm.la	\
				@top_srcdir@/base/tclplus/libtclPlus.la		\
				@top_srcdir@/base/exception/libException.la	\
				@top_srcdir@/base/thread/libdaqthreads.la	\
				@TCL_LDFLAGS@
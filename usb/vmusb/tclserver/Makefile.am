lib_LTLIBRARIES			= libVMUSBControlServer.la

libVMUSBControlServer_la_CXXFLAGS	= @THREADCXX_FLAGS@

libVMUSBControlServer_la_SOURCES	= 	CControlHardware.cpp	\
					CModuleCommand.cpp	\
					TclServer.cpp		\
					CControlModule.cpp	\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CGDG.cpp		\
					CV812.cpp		\
					CVMUSBModule.cpp	\
					CV6533.cpp		\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					CMonCommand.cpp         \
					Server.cpp		\
					ChicoTrigger.cpp        \
					CWatchCommand.cpp

noinst_HEADERS			=	CGDG.h			\
					CV812.h			\
					CV6533.h		\
					CVMUSBModule.h		\
					CSetCommand.h		\
					server.h		\
					CControlHardware.h	\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					CMonCommand.h          \
					TclServer.h		\
					ChicoTrigger.h		\
					CWatchCommand.h

INCLUDES			= 	-I@top_srcdir@/usb/vmusb/vmusb		\
					-I@top_srcdir@/daq/format                 \
					-I@top_srcdir@/usb/vmusb/devices	\
					-I@top_srcdir@/usb/vmusb/threadcomm	\
					-I@top_srcdir@/usb/vmusb/rdothread	\
					-I@top_srcdir@/usb/vmusb/router	\
					-I@top_srcdir@/usb/vmusb	\
					@LIBTCLPLUS_CFLAGS@	\
					-I@top_srcdir@/base/headers \
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/base/os		\
					@TCL_FLAGS@

libVMUSBControlServer_la_LIBADD	= 	@top_builddir@/usb/vmusb/vmusb/libVMUSB.la          \
					@top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
					@top_builddir@/usb/vmusb/threadcomm/libVMUSBThreadComm.la \
					@top_builddir@/usb/vmusb/rdothread/libVMUSBRdoThread.la \
					@top_builddir@/usb/vmusb/router/libVMUSBRouter.la	\
					@LIBTCLPLUS_LDFLAGS@	\
					@top_builddir@/base/exception/libException.la \
					@top_builddir@/base/os/libdaqshm.la	\
					@top_builddir@/base/thread/libdaqthreads.la \
					@TCL_LDFLAGS@ @THREADLD_FLAGS@

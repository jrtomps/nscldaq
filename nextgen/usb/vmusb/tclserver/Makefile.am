lib_LTLIBRARIES			= libControlServer.la

libControlServer_la_CXXFLAGS	= @THREADCXX_FLAGS@

libControlServer_la_SOURCES	= 	CControlHardware.cpp	\
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
					Server.cpp

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
					TclServer.h

INCLUDES			= 	-I$(top_builddir)/usb/vmusb/vmusb		\
					-I$(top_builddir)/usb/vmusb/devices	\
					-I$(top_builddir)/usb/vmusb/threadcomm	\
					-I$(top_builddir)/usb/vmusb/rdothread	\
					-I$(top_builddir)/usb/vmusb/router	\
					-I$(top_builddir)/usb/vmusb	\
					-I$(top_srcdir)/base/tclplus    \
					-I$(top_srcdir)/base/headers \
					-I$(top_srcdir)/base/exception	\
					-I$(top_srcdir)/base/thread	\
					@TCL_FLAGS@

libControlServer_la_LIBADD	= 	$(top_builddir)/usb/vmusb/vmusb/libVMUSB.la          \
					$(top_builddir)/usb/vmusb/devices/libVMUSBDevices.la \
					$(top_builddir)/usb/vmusb/threadcomm/libThreadComm.la \
					$(top_builddir)/usb/vmusb/rdothread/libRdoThread.la \
					$(top_builddir)/usb/vmusb/router/libRouter.la	\
					$(top_srcdir)/base/tclplus/libtclPlus.la \
					$(top_srcdir)/base/exception/libException.la \
					$(top_srcdir)/base/thread/libdaqthreads.la \
					@TCL_LDFLAGS@ @THREADLD_FLAGS@

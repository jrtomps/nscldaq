lib_LTLIBRARIES			= libCCUSBControlServer.la
libCCUSBControlServer_la_SOURCES	= 	CControlHardware.cpp	\
					CModuleCommand.cpp	\
					TclServer.cpp		\
					CControlModule.cpp	\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					DriverTemplate.cpp	\
					Server.cpp		\
					C894.cpp		\
					CPH7106.cpp		\
					CWatchCommand.cpp

noinst_HEADERS			=	CSetCommand.h		\
					server.h		\
					CControlHardware.h	\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					DriverTemplate.h	\
					TclServer.h		\
					C894.h			\
					CPH7106.h		\
					CWatchCommand.h

INCLUDES			= 	-I@top_srcdir@/usb/ccusb		\
					-I@top_srcdir@/daq/format		\
					-I@top_srcdir@/usb/ccusb/ccusb		\
					-I@top_srcdir@/usb/ccusb/devices	\
					-I@top_srcdir@/usb/ccusb/threadcomm	\
					-I@top_srcdir@/usb/ccusb/rdothread	\
					-I@top_srcdir@/usb/ccusb/router	\
					-I@top_srcdir@/base/tclplus	\
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/base/os		\
					@TCL_FLAGS@ @THREADCXX_FLAGS@

libCCUSBControlServer_la_LIBADD	= 	@top_builddir@/usb/ccusb/ccusb/libCCUSB.la          \
					@top_builddir@/usb/ccusb/devices/libCCUSBDevices.la \
					@top_builddir@/usb/ccusb/threadcomm/libCCUSBThreadComm.la \
					@top_builddir@/usb/ccusb/rdothread/libCCUSBRdoThread.la \
					@top_builddir@/usb/ccusb/router/libCCUSBRouter.la	\
					@TCL_LDFLAGS@				\
					@top_builddir@/base/tclplus/libtclPlus.la \
					@top_builddir@/base/os/libdaqshm.la	\
					@top_builddir@/base/thread/libdaqthreads.la	\
					@top_builddir@/base/exception/libException.la  \
					@THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes

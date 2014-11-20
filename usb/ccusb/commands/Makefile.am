lib_LTLIBRARIES		= libCCUSBCommands.la
libCCUSBCommands_la_SOURCES	=	CBeginRun.cpp	\
				CEndRun.cpp	\
				CPauseRun.cpp	\
				CResumeRun.cpp	\
				CInit.cpp	\
				CExit.cpp	\
				tclUtil.cpp	

include_HEADERS		= 	CBeginRun.h	\
				CEndRun.h	\
				CPauseRun.h	\
				CResumeRun.h	\
				CInit.h	\
				CExit.h		\
				tclUtil.h

libCCUSBCommands_la_CPPFLAGS		= 	-I@top_srcdir@/usb/ccusb/rdothread	\
				-I@top_srcdir@/usb/ccusb/router		\
				-I@top_srcdir@/usb/ccusb/daqconfig	\
				-I@top_srcdir@/usb/ccusb/devices	\
				-I@top_srcdir@/usb/ccusb		\
				-I@top_srcdir@/usb/ccusb/ccusb		\
				-I@top_srcdir@/usb/ccusb/tclserver	\
				@LIBTCLPLUS_CFLAGS@	\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/thread		\
				$(TCL_FLAGS) @THREADCXX_FLAGS@

libCCUSBCommands_la_LIBADD	= 	@top_builddir@/usb/ccusb/rdothread/libCCUSBRdoThread.la	\
				@top_builddir@/usb/ccusb/router/libCCUSBRouter.la	\
				@top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la	\
				@top_builddir@/usb/ccusb/tclserver/libCCUSBControlServer.la	\
				@LIBTCLPLUS_LDFLAGS@	\
				@top_builddir@/base/thread/libdaqthreads.la	\
				$(TCL_LDFLAGS)

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes


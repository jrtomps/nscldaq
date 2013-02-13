lib_LTLIBRARIES		= libCCUSBCommands.la
libCCUSBCommands_la_SOURCES	=	CBeginRun.cpp	\
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
				-I@top_srcdir@/usb/ccusb/ccusb		\
				-I@top_srcdir@/base/tclplus		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/thread		\
				$(TCL_FLAGS) @THREADCXX_FLAGS@

libCCUSBCommands_la_LIBADD	= 	@top_builddir@/usb/ccusb/rdothread/libCCUSBRdoThread.la	\
				@top_builddir@/usb/ccusb/threadcomm/libCCUSBThreadComm.la	\
				@top_builddir@/usb/ccusb/router/libCCUSBRouter.la	\
				@top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la	\
				@top_builddir@/base/tclplus/libtclPlus.la 	\
				@top_builddir@/base/thread/libdaqthreads.la	\
			        @top_builddir@/base/exception/libException.la \
				$(TCL_LDFLAGS)

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes


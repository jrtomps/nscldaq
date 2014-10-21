lib_LTLIBRARIES		= libCCUSBRdoThread.la
libCCUSBRdoThread_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp

noinst_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.

INCLUDES		=  -I@top_srcdir@/usb/ccusb \
			-I@top_srcdir@/usb/ccusb/router				\
			-I@top_srcdir@/usb/ccusb/devices			\
			-I@top_srcdir@/usb/ccusb/ccusb				\
			-I@top_srcdir@/usb/ccusb/daqconfig			\
			@TCL_CPPFLAGS@						\
			@LIBTCLPLUS_CFLAGS@					\
			-I@top_srcdir@/base/thread				\
			-I@top_srcdir@/base/os					\
			-I@top_srcdir@/base/headers @THREADCXX_FLAGS@


libCCUSBRdoThread_la_LIBADD	= @top_builddir@/usb/ccusb/devices/libCCUSBDevices.la	\
			  @top_builddir@/usb/ccusb/ccusb/libCCUSB.la		\
			@top_builddir@/base/thread/libdaqthreads.la		\
			@top_builddir@/base/os/libdaqshm.la			\
			@LIBTCLPLUS_LDFLAGS@ @THREADLD_FLAGS@



install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes




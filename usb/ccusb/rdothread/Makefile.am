lib_LTLIBRARIES		= libRdoThread.la
libRdoThread_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp

noinst_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.

INCLUDES		=  -I@top_srcdir@/usb/ccusb/threadcomm \
			-I@top_srcdir@/usb/ccusb/router				\
			-I@top_srcdir@/usb/ccusb/devices			\
			-I@top_srcdir@/usb/ccusb/ccusb				\
			-I@top_srcdir@/base/exception				\
			-I@top_srcdir@/base/thread				\
			-I@top_srcdir@/base/headers


libRdoThread_la_LIBADD	= @top_builddir@/usb/ccusb/threadcomm/libThreadComm.la	\
			  @top_builddir@/usb/ccusb/devices/libCCUSBDevices.la	\
			  @top_builddir@/usb/ccusb/ccusb/libCCUSB.la		\
			@top_builddir@/base/thread/libdaqthreads.la		\
			@top_builddir@/base/exception/libException.la


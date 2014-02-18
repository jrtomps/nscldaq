lib_LTLIBRARIES		= libVMUSBRdoThread.la
libVMUSBRdoThread_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp

libVMUSBRdoThread_CXXFLAGS = @THREADCXX_FLAGS@

noinst_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.

INCLUDES		= -I@top_srcdir@/usb/vmusb/threadcomm 	\
			-I@top_srcdir@/usb/vmusb/router		\
			-I@top_srcdir@/usb/vmusb/devices	\
			-I@top_srcdir@/usb/vmusb/vmusb		\
			-I@top_srcdir@/usb/vmusb/daqconfig	\
			-I@top_srcdir@/usb/vmusb/tclserver @TCL_FLAGS@ \
			@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/base/thread	\
			-I@top_srcdir@/usb/vmusb	\
			-I@top_srcdir@/base/os


libVMUSBRdoThread_la_LIBADD	= @top_builddir@/usb/vmusb/threadcomm/libVMUSBThreadComm.la	\
			  @top_builddir@/usb/vmusb/devices/libVMUSBDevices.la	\
			  @top_builddir@/usb/vmusb/vmusb/libVMUSB.la		\
			@LIBTCLPLUS_LDFLAGS@					\
			@top_builddir@/base/thread/libdaqthreads.la \
			@top_builddir@/base/os/libdaqshm.la	\
			@THREADLD_FLAGS@


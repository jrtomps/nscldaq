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
			@LIBTCLPLUS_CFLAGS@	\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/base/thread	\
			-I@top_srcdir@/usb/vmusb


libVMUSBRdoThread_la_LIBADD	= @top_srcdir@/usb/vmusb/threadcomm/libVMUSBThreadComm.la	\
			  @top_srcdir@/usb/vmusb/devices/libVMUSBDevices.la	\
			  @top_srcdir@/usb/vmusb/vmusb/libVMUSB.la		\
			@LIBEXCEPTION_LDFLAGS@		\
			@top_srcdir@/base/thread/libdaqthreads.la @THREADLD_FLAGS@


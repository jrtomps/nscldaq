lib_LTLIBRARIES		= libRdoThread.la
libRdoThread_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp

include_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.

INCLUDES		= -I@top_srcdir@/usb/vmusb/threadcomm 	\
			-I@top_srcdir@/usb/vmusb/router		\
			-I@top_srcdir@/usb/vmusb/devices	\
			-I@top_srcdir@/usb/vmusb/vmusb		\
			-I@top_srcdir@/usb/vmusb/tclserver @TCL_FLAGS@ \
			-I@top_srcdir@/base/exception	\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/base/thread	\
			-I@top_srcdir@/usb/vmusb


libRdoThread_la_LIBADD	= @top_srcdir@/usb/vmusb/threadcomm/libThreadComm.la	\
			  @top_srcdir@/usb/vmusb/devices/libVMUSBDevices.la	\
			  @top_srcdir@/usb/vmusb/vmusb/libVMUSB.la		\
			@top_srcdir@/base/exception/libException.la             \
			@top_srcdir@/base/thread/libdaqthreads.la


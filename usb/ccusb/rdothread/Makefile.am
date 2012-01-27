lib_LTLIBRARIES		= libRdoThread.la
libRdoThread_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp

include_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.

INCLUDES		= $(SPECTRODAQ_FLAGS) $(NSCLDAQ_FLAGS) -I@top_srcdir@/threadcomm \
			-I@top_srcdir@/router				\
			-I@top_srcdir@/devices				\
			-I@top_srcdir@/ccusb


libRdoThread_la_LIBADD	= @top_srcdir@/threadcomm/libThreadComm.la	\
			  @top_srcdir@/devices/libVMUSBDevices.la	\
			  @top_srcdir@/ccusb/libCCUSB.la		\
			$(SPECTRODAQ_LDFLAGS) $(NSCLDAQ_LIB) -lException


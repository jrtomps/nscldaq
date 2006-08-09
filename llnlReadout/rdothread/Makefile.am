lib_LTLIBRARIES		= libRdoThread.la
libRdoThread_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp

include_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h

INCLUDES		= $(SPECTRODAQ_FLAGS) -I@top_srcdir@/threadcomm \
			-I@top_srcdir@/router				\
			-I@top_srcdir@/devices				\
			-I@top_srcdir@/vmusb


libRdoThread_la_LIBADD	= @top_srcdir@/threadcomm/libThreadComm.la	\
			  @top_srcdir@/devices/libVMUSBDevices.la	\
			  @top_srcdir@/vmusb/libVMUSB.la		\
			$(SPECTRODAQ_LDFLAGS)


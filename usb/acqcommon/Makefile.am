lib_LTLIBRARIES			= 	libusbacqcommon.la

libusbacqcommon_la_SOURCES	=	CControlQueues.cpp		\
					CAcquisitionThread.cpp

noinst_HEADERS			=	CControlQueues.h		\
					CAcquisitionThread.h


INCLUDES			=	-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/usb/threadcom	\
					-I@top_srcdir@/usb/output		\
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/base/headers

libusbacqcommon_la_LDFLAGS		=	@top_srcdir@/base/thread/libdaqthreads.la	\
					@top_srcdir@/usb/threadcom/libThreadComm.la	\
					@top_srcdir@/usb/output/libOutputStage.la	\
					@top_srcdir@/base/exception/libException.la


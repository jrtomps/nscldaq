lib_LTLIBRARIES			= 	libusbacqcommon.la

libusbacqcommon_la_SOURCES	=	CControlQueues.cpp		\
					CAcquisitionThread.cpp		\
					Globals.cpp			\
					CBeginRun.cpp			\
					CEndRun.cpp			\
					CPauseRun.cpp			\
					CResumeRun.cpp

noinst_HEADERS			=	CControlQueues.h		\
					CAcquisitionThread.h		\
					Globals.h			\
					CBeginRun.h			\
					CEndRun.h			\
					CPauseRun.h			\
					CResumeRun.h


INCLUDES			=	-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/usb/threadcom	\
					-I@top_srcdir@/usb/output		\
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/base/tclplus	\
					-I@top_srcdir@/usb/tclcommon	\
					@TCL_FLAGS@

libusbacqcommon_la_LDFLAGS		=	@top_srcdir@/base/thread/libdaqthreads.la	\
					@top_srcdir@/usb/threadcom/libThreadComm.la	\
					@top_srcdir@/usb/output/libOutputStage.la	\
					@top_srcdir@/base/exception/libException.la	\
					@top_srcdir@/base/tclplus/libtclPlus.la		\
					@top_srcdir@/usb/tclcommon/libusbtclcommon.la	\
					@TCL_LDFLAGS@


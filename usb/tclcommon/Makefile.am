lib_LTLIBRARIES			=	libusbtclcommon.la
libusbtclcommon_la_SOURCES	=	CConfiguration.cpp	\
					CItemConfiguration.cpp	\
					CConfigurableObject.cpp \
					tclUtil.cpp

noinst_HEADERS			=	tclUtil.h		\
					CConfiguration.h	\
					CItemConfiguration.h	\
					CConfigurableObject.h

INCLUDES			=	-I@top_srcdir@/base/tclplus	\
					-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/usb/threadcom	\
					-I@top_srcdir@/base/thread	\
					@TCL_FLAGS@

libusbtclcommon_la_LDFLAGS	=	@top_srcdir@/base/tclplus/libtclPlus.la	\
					@top_srcdir@/base/exception/libException.la	\
					@top_srcdir@/usb/threadcom/libThreadComm.la	\
					@top_srcdir@/base/thread/libdaqthreads.la	\
					@TCL_LDFLAGS@ $(THREADLD_LFAGS)

libusbtclcommon_la_CXXFLAGS	=	$(THREADCXX_FLAGS)
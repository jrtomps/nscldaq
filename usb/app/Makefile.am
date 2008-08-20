lib_LTLIBRARIES			=	libusbapplication.la

libusbapplication_la_SOURCES	=	CApplication.cpp 

libusbapplication_la_DEPENDENCIES = usbReadoutOptions.o

noinst_HEADERS			=	CApplication.h

INCLUDES			=	-I@top_srcdir@/base/headers		\
					-I@top_srcdir@/usb/tclcommon		\
					-I@top_srcdir@/usb/tclserver		\
					-I@top_srcdir@/usb/acqcommon		\
					-I@top_srcdir@/usb/output		\
					-I@top_srcdir@/base/tclplus		\
					-I@top_srcdir@/servers/portmanager	\
					-I@top_srcdir@/base/thread		\
					@TCL_FLAGS@

libusbapplication_la_LIBADD		=	usbReadoutOptions.o

libusbapplication_la_LDFLAGS	=	@top_srcdir@/usb/tclcommon/libusbtclcommon.la	\
					@top_srcdir@//usb/tclserver/libControlServer.la	\
					@top_srcdir@/usb/acqcommon/libusbacqcommon.la		\
					@top_srcdir@/usb/output/libOutputStage.la
					@top_srcdir@/base/tclplus/libtclPlus.la		\
					@top_srcdir@/servers/portmanager/libPortManager.la	\
					@top_srcdir@/base/exception/libException.la	\
					@top_srcdir@/base/thread/libdaqthreads.la	\
					@TCL_LDFLAGS@

BUILT_SOURCES			= 	usbReadoutOptions.h

usbReadoutOptions.h:	usbReadout.ggo
	$(GENGETOPT)	<usbReadout.ggo --file=usbReadoutOptions
	$(CC) -c   usbReadoutOptions.c

usbReadoutOptions.o:	usbReadout.ggo
	$(GENGETOPT)	<usbReadout.ggo --file=usbReadoutOptions
	$(CC) -c   usbReadoutOptions.c
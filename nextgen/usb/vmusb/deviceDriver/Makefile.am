lib_LTLIBRARIES			=	libvmUsbSpecific.la

libvmUsbSpecific_la_SOURCES	= 	CVMUSB.cpp CVMUSBReadoutList.cpp CVMUSBDeviceDriver.cpp	\
					C785.cpp CStack.cpp C3804.cpp C3820.cpp

noinst_HEADERS			=	CVMUSB.h CVMUSBReadoutList.h CVMUSBDeviceDriver.h	\
					CVMUSBConfigurableObject.h			\
					C785.h CStack.h C3804.h C3820.h

INCLUDES			=	-I@top_srcdir@/usb/acqcommon	\
					-I@top_srcdir@/usb/threadcom	\
					-I@top_srcdir@/usb/tclcommon	\
					-I@top_srcdir@/usb/output	\
					-I@top_srcdir@/base/thread	\
					@TCL_FLAGS@

libvmUsbSpecific_la_LDFLAGS	=	@top_srcdir@/usb/acqcommon/libusbacqcommon.la	\
					@top_srcdir@/usb/tclcommon/libusbtclcommon.la	\
					@top_srcdir@/usb/threadcom/libThreadComm.la	\
					@top_srcdir@/usb/output/libOutputStage.la	\
					@top_srcdir@/base/thread/libdaqthreads.la	\
					@TCL_LDFLAGS@
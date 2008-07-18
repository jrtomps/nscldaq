lib_LTLIBRARIES			=	libvmUsbSpecific.la

libvmUsbSpecific_la_SOURCES	= 	CVMUSB.cpp CVMUSBReadoutList.cpp CVMUSBDeviceDriver.cpp

noinst_HEADERS			=	CVMUSB.h CVMUSBReadoutList.h CVMUSBDeviceDriver.h

INCLUDES			=	-I@top_srcdir@/usb/acqcommon	\
					-I@top_srcdir@/usb/threadcom	\
					-I@top_srcdir@/usb/output	\
					-I@top_srcdir@/base/thread

libvmUsbSpecific_la_LDFLAGS	=	@top_srcdir@/usb/acqcommon/libusbacqcommon.la	\
					@top_srcdir@/usb/threadcom/libThreadComm.la	\
					@top_srcdir@/usb/output/libOutputStage.la	\
					@top_srcdir@/base/thread/libdaqthreads.la
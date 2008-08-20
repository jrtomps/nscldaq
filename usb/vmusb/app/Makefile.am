bin_PROGRAMS		=	VMUSBReadout

VMUSBReadout_SOURCES	=	App.cpp
noinst_HEADERS		=       App.h


#  Assume we need all header dirs and all libs in the usb/vmusb tree.


INCLUDES		=	-I@top_srcdir@/usb/acqcommon		\
				-I@top_srcdir@/usb/app			\
				-I@top_srcdir@/usb/commands		\
				-I@top_srcdir@/usb/tclcommon		\
				-I@top_srcdir@/usb/tclserver		\
				-I@top_srcdir@/usb/output		\
				-I@top_srcdir@/usb/threadcom		\
				-I@top_srcdir@/usb/vmusb/daqconfig	\
				-I@top_srcdir@/usb/vmusb/deviceDriver	\
				-I@top_srcdir@/base/thread		\
				@TCL_FLAGS@

VMUSBReadout_LDADD	=	@top_srcdir@/usb/acqcommon/libusbacqcommon.la	\
				@top_srcdir@/usb/tclcommon/libusbtclcommon.la	\
				@top_srcdir@/usb/output/libOutputStage.la	\
				@top_srcdir@/usb/threadcom/libThreadComm.la	\
				@top_srcdir@/usb/output/libOutputStage.la	\
				@top_srcdir@/usb/app/libusbapplication.la	\
				@top_srcdir@/usb/tclserver/libControlServer.la	\
				@top_srcdir@/usb/vmusb/deviceDriver/libvmUsbDeviceDrivers.la \
				@top_srcdir@/usb/vmusb/daqconfig/libvmUsbConfigCommands.la	\
				@top_srcdir@/base/thread/libdaqthreads.la	\
				@TCL_LDFLAGS@ -lusb



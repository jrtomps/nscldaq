lib_LTLIBRARIES				= 	libvmUsbConfigCommands.la
libvmUsbConfigCommands_la_SOURCES 	=	CADCCommand.cpp	\
						CCAENChainCommand.cpp	\
						CCAENV830Command.cpp	\
						CMADCCommand.cpp	\
						CMADCScalerCommand.cpp	\
						CMarkerCommand.cpp	\
						CModuleCommand.cpp	\
						CNADC2530Command.cpp	\
						CScalerCommand.cpp	\
						CSIS3804Command.cpp	\
						CStackCommand.cpp	\
						CVMUSBConfig.cpp

noinst_HEADERS				=	CADCCommand.h	\
						CCAENChainCommand.h	\
						CCAENV830Command.h	\
						CMADCCommand.h	\
						CMADCScalerCommand.h	\
						CMarkerCommand.h	\
						CModuleCommand.h	\
						CNADC2530Command.h	\
						CScalerCommand.h	\
						CSIS3804Command.h	\
						CStackCommand.h

INCLUDES				= 	-I@top_srcdir@/base/tclplus	\
						-I@top_srcdir@/usb/tclcommon	\
						-I@top_srcdir@/base/headers	\
						-I@top_srcdir@/usb/vmusb/deviceDriver	\
						-I@top_srcdir@/usb/acqcommon		\
						@TCL_FLAGS@



libvmUsbConfigCommands_la_LDFLAGS	=	@top_srcdir@/base/tclplus/libtclPlus.la	\
						@top_srcdir@/usb/tclcommon/libusbtclcommon.la	\
						@top_srcdir@/usb/vmusb/deviceDriver/libvmUsbDeviceDrivers.la	\
						@top_srcdir@/usb/acqcommon/libusbacqcommon.la			\
						@TCL_LDFLAGS@
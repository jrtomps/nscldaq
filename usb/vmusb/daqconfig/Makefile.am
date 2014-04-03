lib_LTLIBRARIES		= libVMUSBConfig.la
libVMUSBConfig_la_SOURCES	= 	CADCCommand.cpp		\
				CConfiguration.cpp	\
				CScalerCommand.cpp	\
				CCAENChainCommand.cpp	\
				CStackCommand.cpp	\
				CCAENV830Command.cpp	\
				CMarkerCommand.cpp	\
				CSIS3804Command.cpp	\
				CNADC2530Command.cpp	\
				CMADCCommand.cpp	\
				CMADCScalerCommand.cpp	\
				CV1x90Command.cpp	\
				CV977Command.cpp	\
				CMASECommand.cpp	\
				CMADCChainCommand.cpp   \
				CHINPCommand.cpp \
				CPSDCommand.cpp \
				CV1729Command.cpp	\
				CV1495scCommand.cpp	\
				CUserCommand.cpp	\
				CAddTclDriver.cpp	\
				CHiRACommand.cpp	\
				CVMUSBCommand.cpp 

libVMUSBConfig_la_CXXFLAGS=@THREADCXX_FLAGS@


noinst_HEADERS		=	CADCCommand.h  		\
				CConfiguration.h 	\
				CScalerCommand.h	\
				CCAENChainCommand.h	\
				CStackCommand.h		\
				CCAENV830Command.h	\
				CMarkerCommand.h	\
				CSIS3804Command.h	\
				CNADC2530Command.h	\
				CMADCCommand.h		\
				CMADCScalerCommand.h	\
				CV1x90Command.h		\
				CV977Command.h		\
				CMASECommand.h		\
				CMADCChainCommand.h	\
				CHINPCommand.h \
				CPSDCommand.h		\
				CV1729Command.h		\
				CV1495scCommand.h	\
				CUserCommand.h		\
				CAddTclDriver.h		\
				CHiRACommand.h		\
				CVMUSBCommand.h 



INCLUDES		= 	-I@top_srcdir@/usb/vmusb/devices	\
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/usb/vmusb/vmusb		\
				-I@top_srcdir@/usb/common/devices		\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/base/headers		\
				$(TCL_FLAGS) 


libVMUSBConfig_la_LIBADD	= @top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
				@LIBTCLPLUS_LDFLAGS@	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes

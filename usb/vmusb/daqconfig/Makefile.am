lib_LTLIBRARIES		= libConfig.la
libConfig_la_SOURCES	= 	CADCCommand.cpp		\
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
				CV1495scCommand.cpp


libConfig_la_CXXFLAGS=@THREADCXX_FLAGS@


include_HEADERS		=	CADCCommand.h  		\
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
				CV1495scCommand.h




INCLUDES		= 	-I@top_srcdir@/usb/vmusb/devices	\
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/base/tclplus		\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/exception		\
				$(TCL_FLAGS) 


libConfig_la_LIBADD	=	@top_srcdir@/usb/vmusb/devices/libVMUSBDevices.la \
				@top_srcdir@/base/tclplus/libtclPlus.la		\
				@top_srcdir@/base/exception/libException.la	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@


lib_LTLIBRARIES		= libConfig.la
libConfig_la_SOURCES	= 	CConfiguration.cpp	\
				CStackCommand.cpp	\
				CPH7xxCommand.cpp	\
				CAD811Command.cpp	\
				CLRS2551Command.cpp	\
				CLRS2249Command.cpp	\
				CLRS2228Command.cpp	\
				CCAEN257Command.cpp	\
				C1205Command.cpp	\
				CCCUSBCommand.cpp	\
				CPH7132Command.cpp


#				CScalerCommand.cpp


noinst_HEADERS		=	CConfiguration.h 	\
				CScalerCommand.h	\
				CStackCommand.h		\
				CPH7xxCommand.h		\
				CAD811Command.h		\
				CLRS2551Command.h	\
				CLRS2249Command.h	\
				CLRS2228Command.h	\
				CCAEN257Command.h	\
				C1205Command.h		\
				CCCUSBCommand.h		\
				CPH7132Command.h



INCLUDES		= 	-I@top_srcdir@/usb/ccusb/devices	\
				-I@top_srcdir@/base/tclplus		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/base/headers		\
				$(TCL_FLAGS)

libConfig_la_LIBADD	=	@top_builddir@/usb/ccusb/devices/libCCUSBDevices.la 	\
				@top_builddir@/base/tclplus/libtclPlus.la	\
				@top_builddir@/base/exception/libException.la	\
				$(TCL_LDFLAGS)


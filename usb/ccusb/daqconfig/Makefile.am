lib_LTLIBRARIES		= libCCUSBConfig.la
libCCUSBConfig_la_SOURCES	= 	CConfiguration.cpp	\
				CStackCommand.cpp	\
				CPH7xxCommand.cpp	\
				CAD811Command.cpp	\
				CLRS2551Command.cpp	\
				CLRS2249Command.cpp	\
				CLRS2228Command.cpp	\
				CCAEN257Command.cpp	\
				C1205Command.cpp	\
				CCCUSBCommand.cpp	\
				CPH7132Command.cpp	\
				CUserCommand.cpp	\
				CAddTclDriver.cpp


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
				CPH7132Command.h	\
				CUserCommand.h		\
				CAddTclDriver.h



INCLUDES		= 	-I@top_srcdir@/usb/ccusb/devices	\
				-I@top_srcdir@/usb/ccusb		\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/base/headers		\
				$(TCL_FLAGS) @THREADCXX_FLAGS@

libCCUSBConfig_la_LIBADD	=	@top_builddir@/usb/ccusb/devices/libCCUSBDevices.la 	\
				@LIBTCLPLUS_LDFLAGS@	\
				@top_builddir@/base/exception/libException.la	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@


install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes

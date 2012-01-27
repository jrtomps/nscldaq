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


include_HEADERS		=	CConfiguration.h 	\
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



INCLUDES		= 	-I@top_srcdir@/devices	\
				$(NSCLDAQ_FLAGS) $(TCL_FLAGS) \
				$(SPECTRODAQ_FLAGS)

libConfig_la_LIBADD	=	@top_srcdir@/devices/libVMUSBDevices.la 	\
				$(TCL_LDFLAGS) $(SPECTRODAQ_LDFLAGS)


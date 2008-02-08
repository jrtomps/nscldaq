lib_LTLIBRARIES		= libConfig.la
libConfig_la_SOURCES	= 	CConfiguration.cpp	\
				CStackCommand.cpp	\
				CPH7xxCommand.cpp


#				CScalerCommand.cpp


include_HEADERS		=	CConfiguration.h 	\
				CScalerCommand.h	\
				CStackCommand.h		\
				CPH7xxCommand.h



INCLUDES		= 	-I@top_srcdir@/devices	\
				$(NSCLDAQ_FLAGS) $(TCL_FLAGS) \
				$(SPECTRODAQ_FLAGS)

libConfig_la_LIBADD	=	@top_srcdir@/devices/libVMUSBDevices.la 	\
				$(TCL_LDFLAGS) $(SPECTRODAQ_LDFLAGS)


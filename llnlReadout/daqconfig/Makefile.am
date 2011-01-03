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
				CPSDCommand.cpp


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
				CPSDCommand.h




INCLUDES		= 	-I@top_srcdir@/devices	\
				$(NSCLDAQ_FLAGS) $(TCL_FLAGS) \
				$(SPECTRODAQ_FLAGS)

libConfig_la_LIBADD	=	@top_srcdir@/devices/libVMUSBDevices.la 	\
				$(TCL_LDFLAGS) $(SPECTRODAQ_LDFLAGS)


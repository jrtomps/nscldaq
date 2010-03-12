lib_LTLIBRARIES 		= libVMUSBDevices.la
libVMUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp	\
					CReadoutHardware.cpp	\
					CPH7xx.cpp	\
					CAD811.cpp	\
					CStack.cpp	\
					CLRS2551.cpp	\
					CLRS2249.cpp	\
					CLRS2228.cpp	\
					CCAEN257.cpp


INCLUDES	= 	-I$(top_builddir)/ccusb \
			-I$(top_builddir)/daqconfig \
			@TCL_FLAGS@ $(SPECTRODAQ_FLAGS)


libVMUSBDevices_la_LIBADD	= $(top_builddir)/ccusb/libCCUSB.la \
				  @TCL_LDFLAGS@ $(SPECTRODAQ_LDFLAGS)

include_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					CStack.h		\
					CAD811.h		\
					CPH7xx.h		\
					CLRS2551.h		\
					CLRS2249.h		\
					CLRS2228.h		\
					CCAEN257.h



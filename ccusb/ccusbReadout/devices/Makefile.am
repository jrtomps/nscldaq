lib_LTLIBRARIES 		= libVMUSBDevices.la
libVMUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp	\
					CPH7xx.cpp	\
					CStack.cpp


INCLUDES	= 	-I$(top_builddir)/ccusb \
			-I$(top_builddir)/daqconfig \
			@TCL_FLAGS@ $(SPECTRODAQ_FLAGS)


libVMUSBDevices_la_LIBADD	= $(top_builddir)/ccusb/libCCUSB.la \
				  @TCL_LDFLAGS@ $(SPECTRODAQ_LDFLAGS)

include_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					CStack.h		\
					CPH7xx.h



lib_LTLIBRARIES 		= libVMUSBDevices.la
libVMUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp 	\
					C3820.cpp		\
					C785.cpp


INCLUDES	= -I$(top_builddir)/vmusb @TCL_FLAGS@


libVMUSBDevices_la_LIBADD	= $(top_builddir)/vmusb/libVMUSB.la \
				  @TCL_LDFLAGS@

include_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					C3820.h			\
					C785.h



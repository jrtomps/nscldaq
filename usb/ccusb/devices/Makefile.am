lib_LTLIBRARIES 		= libCCUSBDevices.la
libCCUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp	\
					CReadoutHardware.cpp	\
					CPH7xx.cpp	\
					CAD811.cpp	\
					CStack.cpp	\
					CLRS2551.cpp	\
					CLRS2249.cpp	\
					CLRS2228.cpp	\
					CCAEN257.cpp	\
					CC1205.cpp	\
					CCCUSBModule.cpp	\
					CPH7132.cpp	\
					CTclModule.cpp


INCLUDES	= 	-I$(top_srcdir)/usb/ccusb/ccusb \
			-I$(top_srcdir)/usb/ccusb/daqconfig \
			-I$(top_srcdir)/usb/ccusb		\
			@LIBTCLPLUS_CFLAGS@		\
			-I$(top_srcdir)/base/headers		\
			@TCL_FLAGS@ @THREADCXX_FLAGS@


libCCUSBDevices_la_LIBADD	= $(top_builddir)/usb/ccusb/ccusb/libCCUSB.la \
				@LIBTCLPLUS_LDFLAGS@	\
				  @TCL_LDFLAGS@ @THREADLD_FLAGS@

noinst_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					CStack.h		\
					CAD811.h		\
					CPH7xx.h		\
					CLRS2551.h		\
					CLRS2249.h		\
					CLRS2228.h		\
					CCAEN257.h		\
					CC1205.h		\
					CCCUSBModule.h		\
					CPH7132.h		\
					CTclModule.h


install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes

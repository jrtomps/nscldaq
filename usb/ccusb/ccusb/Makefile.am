lib_LTLIBRARIES		= libCCUSB.la
libCCUSB_la_SOURCES	= CCCUSB.cpp CCCUSBReadoutList.cpp
include_HEADERS         = CCCUSB.h   CCCUSBReadoutList.h

INCLUDES		= $(USBSWITCHES) @THREADCXX_FLAGS@

libCCUSB_la_LIBADD	= $(USB_LIBS)   @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes
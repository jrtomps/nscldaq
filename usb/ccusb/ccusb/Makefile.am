lib_LTLIBRARIES		= libCCUSB.la libCCCUSB.la
libCCUSB_la_SOURCES	= CCCUSB.cpp CCCUSBReadoutList.cpp
include_HEADERS         = CCCUSB.h   CCCUSBReadoutList.h

libCCCUSB_la_SOURCES    = CCCUSB_wrap.cxx

BUILT_SOURCES=CCCUSB_wrap.cxx

CCCUSB_wrap.cxx:	CCCUSB.h
	swig -c++ -debug-classes  -tcl  -namespace -module CCCUSB  CCCUSB.h


INCLUDES		= @TCL_FLAGS@ @USBSWITCHES@ @THREADCXX_FLAGS@

libCCUSB_la_LIBADD	= @USB_LIBS@   @THREADLD_FLAGS@

libCCCUSB_la_LIBADD     = libCCUSB.la @THREADLD_FLAGS@ 

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes
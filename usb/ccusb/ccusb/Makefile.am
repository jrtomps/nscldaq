lib_LTLIBRARIES		= libCCUSB.la libCCCUSB.la libCCCUSBReadoutList.la
libCCUSB_la_SOURCES	= CCCUSB.cpp CCCUSBReadoutList.cpp
include_HEADERS         = CCCUSB.h   CCCUSBReadoutList.h

libCCCUSB_la_SOURCES    = CCCUSB_wrap.cxx
libCCCUSBReadoutList_la_SOURCES = CCCUSBReadoutList_wrap.cxx

 BUILT_SOURCES=CCCUSB_wrap.cxx CCCUSBReadoutList_wrap.cxx

 CCCUSB_wrap.cxx:	CCCUSB.h
	 swig -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  CCCUSB.h

 CCCUSBReadoutList_wrap.cxx: CCCUSBReadoutList.h
	 swig -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  CCCUSBReadoutList.h


 INCLUDES		= @TCL_FLAGS@ @USBSWITCHES@ @THREADCXX_FLAGS@

 libCCUSB_la_LIBADD	= @USB_LIBS@   @THREADLD_FLAGS@

 libCCCUSB_la_LIBADD     = libCCUSB.la @THREADLD_FLAGS@ 

 libCCCUSBReadoutList_la_LIBADD = libCCUSB.la @THREADLD_FLAGS@ 

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  *.h @prefix@/ccusbdriver/includes

install-exec-local:
	(cd $(libdir); echo pkg_mkIndex . libCCCUSB.so libCCCUSBReadoutList.so | tclsh)

clean-local:
	$(RM) -f CCCUSB_wrap.cxx
	$(RM) -f CCCUSBReadoutList_wrap.cxx


lib_LTLIBRARIES		= libCCUSB.la libCCCUSB.la libCCCUSBReadoutList.la
libCCUSB_la_SOURCES	= CCCUSB.cpp CCCUSBReadoutList.cpp
include_HEADERS         = CCCUSB.h   CCCUSBReadoutList.h

libCCCUSB_la_SOURCES    = @srcdir@/CCCUSB_wrap.cxx
libCCCUSBReadoutList_la_SOURCES = @srcdir@/CCCUSBReadoutList_wrap.cxx

 BUILT_SOURCES=@srcdir@/CCCUSB_wrap.cxx @srcdir@/CCCUSBReadoutList_wrap.cxx

 @srcdir@/CCCUSB_wrap.cxx:	 @srcdir@/CCCUSB.h
	 @SWIG@ -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  -outdir @srcdir@ \
		@srcdir@/CCCUSB.h

  @srcdir@/CCCUSBReadoutList_wrap.cxx:  @srcdir@/CCCUSBReadoutList.h
	 @SWIG@ -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  -outdir @srcdir@ \
		@srcdir@/CCCUSBReadoutList.h


 INCLUDES		= -I@top_srcdir@/base/os \
		@TCL_FLAGS@ @USBSWITCHES@ @THREADCXX_FLAGS@

 libCCUSB_la_LIBADD	= @top_builddir@/base/os/libdaqshm.la \
	 @USB_LIBS@   @THREADLD_FLAGS@

 libCCCUSB_la_LIBADD     = libCCUSB.la @top_builddir@/base/os/libdaqshm.la \
			@THREADLD_FLAGS@ 

 libCCCUSBReadoutList_la_LIBADD = libCCUSB.la @top_builddir@/base/os/libdaqshm.la \
			@THREADLD_FLAGS@ 

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
	echo "package ifneeded cccusb 1.0 [list load [file join \$$dir libCCCUSB.so]]" >> @libdir@/pkgIndex.tcl
	echo "package ifneeded cccusbreadoutlist 1.0 [list load [file join \$$dir libCCCUSBReadoutList.so]]" >> @libdir@/pkgIndex.tcl



clean-local:
	$(RM) -f $(BUILT_SOURCES)



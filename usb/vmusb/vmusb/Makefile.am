lib_LTLIBRARIES		= libVMUSB.la libVMUSBRemote.la libCVMUSB.la \
	libCVMUSBReadoutList.la

libVMUSB_la_SOURCES 	=  CVMUSBusb.cpp CVMUSBReadoutList.cpp CVMUSBEthernet.cpp \
	CVMUSBFactory.cpp \
	CVMUSB.cpp


libVMUSBRemote_la_SOURCES = CVMUSBReadoutList.cpp \
	CVMUSBRemote.cpp \
	CVMUSBEthernet.cpp




noinst_HEADERS		= CVMUSB.h   \
	CVMUSBReadoutList.h  \
	CVMUSBusb.h	\
	CVMUSBEthernet.h \
	CVMUSBRemote.h       \
	CVMUSBFactory.h






libVMUSB_CXXFLAGS=@THREADCXX_FLAGS@

INCLUDES		= $(USBSWITCHES) -I$(top_srcdir)/base/headers -I$(top_srcdir)/base/tclplus  \
			-I@top_srcdir@/base/tcpip \
			 -I$(top_srcdir)/base/exception @TCL_FLAGS@

libVMUSB_la_LIBADD       = @top_srcdir@/base/tcpip/libTcp.la $(USB_LIBS)  

libVMUSBRemote_la_LIBADD = @top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
	@TCL_LDFLAGS@ \
	@top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/base/exception/libException.la \
	@top_builddir@/base/tclplus/libtclPlus.la -lException @THREADLD_FLAGS@

#IVP tests:


TEST_MODULES = TestRunner.cpp regTests.cpp  vmeTests.cpp


tests: $(TEST_MODULES)
	$(CXX) -g -o tests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	$(TEST_MODULES) $(CPPUNIT_LDFLAGS) -L$(prefix)/lib -lVMUSB \
	-Wl,"-rpath=$(prefix)/lib"

installcheck-local: tests
	./tests



install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  CVMUSB*.h @prefix@/vmusbdriver/includes
	echo "Registering swig packages with lib's pkgIndex.tcl"
	echo "package ifneeded cvmusbreadoutlist 1.0 [list load [file join \$$dir libCVMUSBReadoutList.so]]" >> @libdir@/pkgIndex.tcl
	echo "package ifneeded cvmusb 1.0 [list load [file join \$$dir libCVMUSB.so]]" >> @libdir@/pkgIndex.tcl


clean-local:
	rm -f $(BUILT_SOURCES)

#  Stuff that needs to go int the distro:


EXTRA_DIST = $(TEST_MODULES)

# Swig interface generation:

libCVMUSB_la_SOURCES = CVMUSB_wrap.cxx
libCVMUSB_la_LIBADD  = libVMUSB.la @THREADLD_FLAGS@

BUILT_SOURCES=CVMUSB_wrap.cxx CVMUSBReadoutList_wrap.cxx

CVMUSB_wrap.cxx:	CVMUSB.h
	swig -c++ -debug-classes -tcl -namespace -pkgversion 1.0 CVMUSB.h

libCVMUSBReadoutList_la_SOURCES = CVMUSBReadoutList_wrap.cxx
libCVMUSBReadoutList_la_LIBADD = libVMUSB.la @THREADLD_FLAGS@


CVMUSBReadoutList_wrap.cxx: CVMUSBReadoutList.h
	swig -c++ -debug-classes -tcl -namespace -pkgversion 1.0 CVMUSBReadoutList.h

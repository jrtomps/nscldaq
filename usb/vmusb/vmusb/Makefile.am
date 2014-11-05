lib_LTLIBRARIES		= libVMUSB.la libVMUSBRemote.la libCVMUSB.la \
	libCVMUSBReadoutList.la

COMPILATION_FLAGS	= $(USBSWITCHES) -I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@  \
			-I@top_srcdir@/base/tcpip \
			-I@top_srcdir@/base/os	\
			  @TCL_FLAGS@ @THREADCXX_FLAGS@

libVMUSB_la_SOURCES 	=  CVMUSBusb.cpp CVMUSBReadoutList.cpp CVMUSBEthernet.cpp \
	CVMUSBFactory.cpp \
	CVMUSB.cpp \
	CMockVMUSB.cpp

libVMUSB_la_CPPFLAGS=$(COMPILATION_FLAGS)

libVMUSBRemote_la_SOURCES = CVMUSBReadoutList.cpp \
	CVMUSBRemote.cpp \
	CVMUSBEthernet.cpp

libVMUSBRemote_la_CPPFLAGS=$(COMPILATION_FLAGS)


noinst_HEADERS		= CVMUSB.h   \
	CVMUSBReadoutList.h  \
	CVMUSBusb.h	\
	CVMUSBEthernet.h \
	CVMUSBRemote.h       \
	CVMUSBFactory.h \
	CMockVMUSB.h


libVMUSB_la_LIBADD       = @top_builddir@/base/tcpip/libTcp.la \
			@LIBTCLPLUS_LDFLAGS@			\
			@top_builddir@/base/os/libdaqshm.la	\
				$(USB_LIBS)  

libVMUSBRemote_la_LIBADD = @top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
	@TCL_LDFLAGS@ \
	@top_builddir@/base/tcpip/libTcp.la \
	@LIBEXCEPTION_LDFLAGS@  @THREADLD_FLAGS@

#IVP tests:


TEST_MODULES = TestRunner.cpp regTests.cpp  vmeTests.cpp


tests: $(TEST_MODULES)
	$(CXX) -g -o tests -I. -I@prefix@/include $(CPPUNIT_INCLUDES) \
	$(TEST_MODULES) $(CPPUNIT_LDFLAGS) -L@prefix@/lib -lVMUSB \
	-Wl,"-rpath=@prefix@/lib"

installcheck-local: tests
	./tests



install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	for f in @srcdir@/CVMUSB*.h; do $(INSTALL_DATA)  $$f  @prefix@/vmusbdriver/includes; done
	echo "Registering swig packages with lib's pkgIndex.tcl"
	echo "package ifneeded cvmusbreadoutlist 1.0 [list load [file join \$$dir libCVMUSBReadoutList.so]]" >> @libdir@/pkgIndex.tcl
	echo "package ifneeded cvmusb 1.0 [list load [file join \$$dir libCVMUSB.so]]" >> @libdir@/pkgIndex.tcl


clean-local:
	rm -f $(BUILT_SOURCES)

#  Stuff that needs to go int the distro:


EXTRA_DIST = $(TEST_MODULES)

# Swig interface generation:

libCVMUSB_la_SOURCES = @srcdir@/CVMUSB_wrap.cxx
libCVMUSB_la_LIBADD  = libVMUSB.la \
			@top_builddir@/base/os/libdaqshm.la	\
			@THREADLD_FLAGS@

libCVMUSB_la_CPPFLAGS=$(COMPILATION_FLAGS)

BUILT_SOURCES=@srcdir@/CVMUSB_wrap.cxx @srcdir@/CVMUSBReadoutList_wrap.cxx

@srcdir@/CVMUSB_wrap.cxx:	@srcdir@/CVMUSB.h
	@SWIG@ -c++ -outdir @srcdir@ -debug-classes -tcl -namespace -pkgversion 1.0 @srcdir@/CVMUSB.h 

libCVMUSBReadoutList_la_SOURCES = @srcdir@/CVMUSBReadoutList_wrap.cxx
libCVMUSBReadoutList_la_LIBADD = libVMUSB.la \
				@top_builddir@/base/os/libdaqshm.la	\
				@THREADLD_FLAGS@

libCVMUSBReadoutList_la_CPPFLAGS=$(COMPILATION_FLAGS)


@srcdir@/CVMUSBReadoutList_wrap.cxx: @srcdir@/CVMUSBReadoutList.h
	@SWIG@ -c++ -outdir @srcdir@ -debug-classes -tcl -namespace -pkgversion 1.0 \
	 @srcdir@/CVMUSBReadoutList.h 

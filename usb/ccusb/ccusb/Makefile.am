lib_LTLIBRARIES		= libCCUSB.la libCCCUSB.la libCCCUSBReadoutList.la
COMPILATION_FLAGS		= -I@top_srcdir@/base/os \
												@LIBTCLPLUS_CFLAGS@ \
												@TCL_FLAGS@ \
												@USBSWITCHES@ \
												@THREADCXX_FLAGS@ \
												-I@top_srcdir@/base/tcpip 


libCCUSB_la_SOURCES	= CCCUSB.cpp CCCUSBusb.cpp CCCUSBRemote.cpp CMockCCUSB.cpp CCCUSBReadoutList.cpp
libCCUSB_la_CPPFLAGS    = $(COMPILATION_FLAGS)
include_HEADERS         = CCCUSB.h CCCUSBusb.h CCCUSBRemote.h CMockCCUSB.h CCCUSBReadoutList.h


## SWIG wrapper generation

libCCCUSB_la_SOURCES    = @srcdir@/CCCUSB_wrap.cxx
libCCCUSB_la_CPPFLAGS=$(COMPILATION_FLAGS)

libCCCUSBReadoutList_la_SOURCES = @srcdir@/CCCUSBReadoutList_wrap.cxx
libCCCUSBReadoutList_la_CPPFLAGS= $(COMPILATION_FLAGS)

BUILT_SOURCES=@srcdir@/CCCUSB_wrap.cxx @srcdir@/CCCUSBReadoutList_wrap.cxx

@srcdir@/CCCUSB_wrap.cxx:	 @srcdir@/CCCUSB.h
	@SWIG@ -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  -outdir @srcdir@ \
						@srcdir@/CCCUSB.i

@srcdir@/CCCUSBReadoutList_wrap.cxx:  @srcdir@/CCCUSBReadoutList.h
		@SWIG@ -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  -outdir @srcdir@ \
						@srcdir@/CCCUSBReadoutList.i



## Normal C++ library containing CCCUSB*
libCCUSB_la_LIBADD	= @top_builddir@/base/os/libdaqshm.la \
										@TCL_LDFLAGS@ \
                    @LIBTCLPLUS_LDFLAGS@ \
										@top_builddir@/base/tcpip/libTcp.la \
										@LIBEXCEPTION_LDFLAGS@  @THREADLD_FLAGS@ \
	  								@USB_LIBS@  

## SWIG Wrapper containing cvmusb::CCCUSB 
libCCCUSB_la_LIBADD = libCCUSB.la @top_builddir@/base/os/libdaqshm.la \
										@TCL_LDFLAGS@ \
                    @LIBTCLPLUS_LDFLAGS@ \
										@top_builddir@/base/tcpip/libTcp.la \
										@LIBEXCEPTION_LDFLAGS@ 

## SWIG Wrapper containing cvmusbreadoutlist::CCCUSBReadoutList
libCCCUSBReadoutList_la_LIBADD = libCCUSB.la @top_builddir@/base/os/libdaqshm.la \
			@THREADLD_FLAGS@ 

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
	echo "package ifneeded cccusb 1.0 [list load [file join \$$dir libCCCUSB.so]]" >> @libdir@/pkgIndex.tcl
	echo "package ifneeded cccusbreadoutlist 1.0 [list load [file join \$$dir libCCCUSBReadoutList.so]]" >> @libdir@/pkgIndex.tcl



## -------------------------------------------
##          TESTS
## -------------------------------------------

noinst_PROGRAMS = unittests tests remotetests

## These all link against installed libraries rather than
## the .la files produced before the installation. These are
## supposed to be production test programs.

## Manual test programs for CCUSBusb
tests_SOURCES = TestRunner.cpp \
								regTests.cpp
tests_LDADD = $(CPPUNIT_LDFLAGS) \
							-L@libdir@ -lCCUSB \
							@TCL_LDFLAGS@ \
							@LIBTCLPLUS_LDFLAGS@
tests_CXXFLAGS = $(AM_CXXFLAGS)
tests_LDFLAGS = -Wl,"-rpath=@libdir@" 


## Manual test program for CCCUSBRemote
remotetests_SOURCES = TestRunner.cpp \
											remoteRegTests.cpp
remotetests_LDADD = $(CPPUNIT_LDFLAGS) \
											-L@libdir@ -lCCUSB \
											@TCL_LDFLAGS@ \
											@LIBTCLPLUS_LDFLAGS@
remotetests_CXXFLAGS = $(AM_CXXFLAGS)
remotetests_LDFLAGS = -Wl,"-rpath=@libdir@" 


## These are tests that run automatically at check-TESTS
unittests_SOURCES  = TestRunner.cpp rdolistTests.cpp
unittests_LDADD    = $(CPPUNIT_LDFLAGS) \
											-L@libdir@ -lCCUSB \
											@TCL_LDFLAGS@ \
											@LIBTCLPLUS_LDFLAGS@
unittests_CXXFLAGS = $(AM_CXXFLAGS)
unittests_LDFLAGS  = -Wl,"-rpath=@libdir@" 


TESTS = ./unittests


clean-local:
	$(RM) -f $(BUILT_SOURCES)


EXTRA_DIST=CCCUSB.i CCCUSBReadoutList.i Asserts.h

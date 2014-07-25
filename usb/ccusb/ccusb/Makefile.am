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

noinst_PROGRAMS = unittests tests

## These are tests run only by hand
tests_SOURCES = TestRunner.cpp \
             regTests.cpp
tests_LDADD = $(CPPUNIT_LDFLAGS) \
              -L@libdir@ -lCCUSB \
	            @TCL_LDFLAGS@ \
              @LIBTCLPLUS_LDFLAGS@
tests_CXXFLAGS = $(AM_CXXFLAGS)
tests_LDFLAGS = -Wl,"-rpath=@libdir@" 

#tests: $(TEST_MODULES)
#	$(CXX) -g -o tests -I@srcdir@ -I@prefix@/include $(CPPUNIT_INCLUDES) \
#	$(TEST_MODULES) $(CPPUNIT_LDFLAGS) -L@prefix@/lib -lCCUSB \
#	-Wl,"-rpath=@prefix@/lib" @TCL_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@
#

## These are tests that run automatically at check-TESTS
unittests_SOURCES  = TestRunner.cpp rdolistTests.cpp
unittests_LDADD    = $(CPPUNIT_LDFLAGS) \
                     -L@libdir@ -lCCUSB \
	                   @TCL_LDFLAGS@ \
                     @LIBTCLPLUS_LDFLAGS@
unittests_CXXFLAGS = $(AM_CXXFLAGS)
unittests_LDFLAGS  = -Wl,"-rpath=@libdir@" 


TESTS = ./unittests


#unittests: $(UNITTESTS_MODULES)
#	$(CXX) -g -o unittests -I@srcdir@ -I@prefix@/include $(CPPUNIT_INCLUDES) \
#	$(UNITTESTS_MODULES) $(CPPUNIT_LDFLAGS) -L@prefix@/lib -lCCUSB \
#	-Wl,"-rpath=@prefix@/lib" @TCL_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@
#
#check-TESTS:
#	./unittests 

#EXTRA_DIST=TestRunner.cpp $(TESTS) $(UNITTESTS)

clean-local:
	$(RM) -f $(BUILT_SOURCES)



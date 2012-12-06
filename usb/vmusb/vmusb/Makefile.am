lib_LTLIBRARIES		= libVMUSB.la libVMUSBRemote.la libCVMUSB.la libCVMUSBReadoutList.la

libVMUSB_la_SOURCES 	= CVMUSB.cpp CVMUSBReadoutList.cpp


libVMUSBRemote_la_SOURCES = CVMUSBReadoutList.cpp \
	CVMUSBRemote.cpp \
	CSocket.cpp	\
	CTCPBadSocketState.cpp \
	CTCPConnectionFailed.cpp \
	CTCPConnectionLost.cpp \
	CTCPNoSuchHost.cpp \
	CTCPNoSuchService.cpp



noinst_HEADERS		= CVMUSB.h   \
	CVMUSBReadoutList.h  \
	CVMUSBRemote.h       \
	CSocket.h		\
	CTCPBadSocketState.h	\
	CTCPConnectionFailed.h	\
	CTCPConnectionLost.h	\
	CTCPNoSuchHost.h	\
	CTCPNoSuchService.h




libVMUSB_CXXFLAGS=@THREADCXX_FLAGS@

INCLUDES		= $(USBSWITCHES) -I$(top_srcdir)/base/headers -I$(top_srcdir)/base/tclplus \
			 -I$(top_srcdir)/base/exception @TCL_FLAGS@

libVMUSB_la_LIBADD       = $(USB_LIBS)  

libVMUSBRemote_la_LIBADD = @TCL_LDFLAGS@ \
	$(top_srcdir)/base/exception/libException.la \
	$(top_srcdir)/base/tclplus/libtclPlus.la -lException @THREADLD_FLAGS@

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

lib_LTLIBRARIES		= libVMUSB.la libVMUSBRemote.la
libVMUSB_la_SOURCES 	= CVMUSB.cpp CVMUSBReadoutList.cpp

libVMUSBRemote_la_SOURCES = CVMUSBReadoutList.cpp \
	CVMUSBRemote.cpp \
	CSocket.cpp	\
	CTCPBadSocketState.cpp \
	CTCPConnectionFailed.cpp \
	CTCPConnectionLost.cpp \
	CTCPNoSuchHost.cpp \
	CTCPNoSuchService.cpp



include_HEADERS		= CVMUSB.h   \
	CVMUSBReadoutList.h  \
	CVMUSBRemote.h       \
	CSocket.h		\
	CTCPBadSocketState.h	\
	CTCPConnectionFailed.h	\
	CTCPConnectionLost.h	\
	CTCPNoSuchHost.h	\
	CTCPNoSuchService.h



INCLUDES		= $(USBSWITCHES) @TCL_FLAGS@ @NSCLDAQ_FLAGS@

libVMUSB_la_LIBADD       = $(USB_LIBS)  

libVMUSBRemote_la_LIBADD = @TCL_LDFLAGS@ @NSCLDAQ_LIB@ -lTclPlus -lException

#IVP tests:


TEST_MODULES = TestRunner.cpp regTests.cpp  vmeTests.cpp


tests: $(TEST_MODULES)
	$(CXX) -g -o tests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	$(TEST_MODULES) $(CPPUNIT_LDFLAGS) -L$(prefix)/lib -lVMUSB \
	-Wl,"-rpath=$(prefix)/lib"

installcheck-local: tests
	./tests



#  Stuff that needs to go int the distro:

EXTRA_DIST = $(TEST_MODULES)
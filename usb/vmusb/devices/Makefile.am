lib_LTLIBRARIES 		= libVMUSBDevices.la
libVMUSBDevices_la_SOURCES	= 	CConfigurableObject.cpp	\
					CReadoutModule.cpp 	\
					C3820.cpp		\
					C785.cpp		\
					CCAENChain.cpp		\
					CStack.cpp		\
					C830.cpp		\
					CMarker.cpp		\
					C3804.cpp		\
					CNADC2530.cpp		\
					CMADC32.cpp		\
					CMADCScaler.cpp		\
					CV1x90.cpp		\
					CV977.cpp		\
					CXLM.cpp		\
					CMASE.cpp		\
					CMADCChain.cpp	\
					CHINP.cpp \
					CPSD.cpp	\
					CV1729.cpp	\
					CV1495sc.cpp	\
					CTclModule.cpp \
					CHiRA.cpp	\
					CVMUSBControl.cpp


INCLUDES	= 	-I@top_srcdir@/usb/vmusb/vmusb 	\
			-I@top_srcdir@/usb/vmusb/daqconfig 	\
			-I@top_srcdir@/usb/vmusb		\
			@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/headers		\
			-I@top_srcdir@/base/os			\
			@TCL_FLAGS@ 

libVMUSBDevices_la_CXXFLAGS=@THREADCXX_FLAGS@


libVMUSBDevices_la_LIBADD	= $(top_builddir)/usb/vmusb/vmusb/libVMUSB.la \
				@top_builddir@/base/os/libdaqshm.la	\
					@LIBTCLPLUS_LDFLAGS@	\
				  @TCL_LDFLAGS@ \
				-lm @THREADLD_FLAGS@

noinst_HEADERS			= 	CConfigurableObject.h	\
					CReadoutModule.h 	\
					CReadoutHardware.h 	\
					C3820.h			\
					C785.h			\
					CCAENChain.h		\
					CStack.h		\
					C830.h			\
					CMarker.h		\
					C3804.h			\
					CNADC2530.h		\
					CMADC32.h		\
					CMADCScaler.h		\
					CV1x90.h		\
					CV977.h			\
					CXLM.h			\
					CMASE.h			\
					CMADCChain.h		\
					MADC32Registers.h	\
					CHINP.h \
					CPSD.h			\
					CV1729.h		\
					CV1495sc.h		\
					CCAENV1x90Opcodes.h	\
					CCAENV1x90Registers.h	\
					CTclModule.h		\
					CHiRA.h			\
					CVMUSBControl.h



install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes


tests: TestRunner.cpp $(TESTS)
	$(CXX) -g -o tests -I. -I$(prefix)/include $(CPPUNIT_INCLUDES) \
	TestRunner.cpp  $(TESTS) -L$(prefix)/lib -lVMUSBDevices -lVMUSB -lConfig \
	$(NSCLDAQ_LIB) @LIBTCLPLUS_LDFLAGS@  $(TCL_LDFLAGS) \
	-Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" \
	$(CPPUNIT_LDFLAGS)


EXTRA_DIST=TestRunner.cpp $(TESTS)
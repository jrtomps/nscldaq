lib_LTLIBRARIES			= libCCUSBControlServer.la
libCCUSBControlServer_la_SOURCES	= 	CControlHardware.cpp	\
					CModuleCommand.cpp	\
					TclServer.cpp		\
					CControlModule.cpp	\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					DriverTemplate.cpp	\
					Server.cpp		\
					C894.cpp		\
					CPH7106.cpp		\
					CWatchCommand.cpp	\
					CModuleFactory.cpp	\
					CModuleCreator.cpp	\
					CCAEN894Creator.cpp     \
					CPH7106Creator.cpp	\
					CTclControlModule.cpp	\
					CTclModuleCreator.cpp \
					CCCUSBControl.cpp	\
					CCCUSBControlCreator.cpp	

noinst_HEADERS			=	CSetCommand.h		\
					server.h		\
					CControlHardware.h	\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					DriverTemplate.h	\
					TclServer.h		\
					C894.h			\
					CPH7106.h		\
					CWatchCommand.h		\
					CModuleFactory.h	\
					CModuleCreator.h	\
					CCAEN894Creator.h	\
					CPH7106Creator.h	\
					CTclControlModule.h	\
					CTclModuleCreator.h \
					CCCUSBControl.h \
          CCCUSBControlCreator.h 


libCCUSBControlServer_la_CPPFLAGS			= 	-I@top_srcdir@/usb/ccusb		\
					-I@top_srcdir@/daq/format		\
					-I@top_srcdir@/usb/ccusb/ccusb		\
					-I@top_srcdir@/usb/ccusb/devices	\
					-I@top_srcdir@/usb/ccusb/rdothread	\
					-I@top_srcdir@/usb/ccusb/router	\
					@LIBTCLPLUS_CFLAGS@	\
					-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/base/os		\
					@TCL_FLAGS@ @THREADCXX_FLAGS@

libCCUSBControlServer_la_LIBADD	= 	@top_builddir@/usb/ccusb/ccusb/libCCUSB.la          \
					@top_builddir@/usb/ccusb/devices/libCCUSBDevices.la \
					@top_builddir@/usb/ccusb/rdothread/libCCUSBRdoThread.la \
					@top_builddir@/usb/ccusb/router/libCCUSBRouter.la	\
					@TCL_LDFLAGS@				\
					@LIBTCLPLUS_LDFLAGS@		\
					@top_builddir@/base/os/libdaqshm.la	\
					@top_builddir@/base/thread/libdaqthreads.la	\
					@THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(mkinstalldirs) @prefix@/ccusbdriver/example
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/sampleDriver.cpp @prefix@/ccusbdriver/example
	$(INSTALL_DATA)  @builddir@/driver_Makefile @prefix@/ccusbdriver/example/Makefile

EXTRA_DIST=driver_Makefile.in sampleDriver.cpp

#noinst_PROGRAMS=unittests
#
#
#unittests_SOURCES=TestRunner.cpp cccusbcontroltests.cpp
#unittests_LDADD    = $(CPPUNIT_LDFLAGS) \
#                     -L@libdir@ -lCCUSBControlServer \
#                     @top_builddir@/usb/ccusb/ccusb/libCCUSB.la \
#                     @top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la \
#	                   @TCL_LDFLAGS@ \
#                     @LIBTCLPLUS_LDFLAGS@
#unittests_CXXFLAGS = $(AM_CXXFLAGS) -I@top_srcdir@/usb/ccusb
#unittests_LDFLAGS  = -Wl,"-rpath=@libdir@" 
#
#
#TESTS = ./unittests



SUBDIRS = 	ccusb 		\
		devices		\
		router		\
		rdothread	\
		tclserver		\
		daqconfig	\
		commands	\
		driverkit   \
    tcldrivers \
    controlpanels \
    tclpackage \
		devtests \
		tclservertests


bin_PROGRAMS = CCUSBReadout

CCUSBReadout_SOURCES = CTheApplication.cpp Globals.cpp @srcdir@/cmdline.c

noinst_HEADERS = CTheApplication.h Globals.h Events.h

CCUSBReadout_CPPFLAGS = -I@top_srcdir@/usb/ccusb/ccusb -I@top_srcdir@/usb/ccusb/devices \
	-I@top_srcdir@/usb/ccusb/router \
	-I@top_srcdir@/usb/ccusb/rdothread \
	 -I@top_srcdir@/usb/ccusb/daqconfig -I@top_srcdir@/usb/ccusb/commands \
	-I@top_srcdir@/usb/ccusb/tclserver  \
	-I@top_srcdir@/usb/common/slowcontrols  \
	-I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/thread \
	-I@top_srcdir@/base/dataflow	                       \
        -I@top_srcdir@/servers/portmanager \
	$(TCL_FLAGS) @THREADCXX_FLAGS@ 


CCUSBReadout_LDADD	=  @top_builddir@/usb/ccusb/devices/libCCUSBDevices.la	\
	@top_builddir@/usb/ccusb/commands/libCCUSBCommands.la	\
	@top_builddir@/usb/ccusb/daqconfig/libCCUSBConfig.la		\
	@top_builddir@/usb/ccusb/rdothread/libCCUSBRdoThread.la	\
	@top_builddir@/usb/ccusb/commands/libCCUSBCommands.la		\
	@top_builddir@/usb/ccusb/router/libCCUSBRouter.la		\
	@top_builddir@/usb/ccusb/ccusb/libCCUSB.la		\
	@top_builddir@/usb/ccusb/tclserver/libCCUSBControlServer.la \
        @top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/thread/libdaqthreads.la		\
	@LIBTCLPLUS_LDFLAGS@	\
	@TCL_LDFLAGS@ @THREADLD_FLAGS@ 

CCUSBReadout_LDFLAGS=-Wl,"-rpath=$(libdir)"

CCUSBReadout_CXXFLAGS= -g

BUILT_SOURCES = @srcdir@/cmdline.c

@srcdir@/cmdline.c: @srcdir@/cmdline.h

@srcdir@/cmdline.h: @srcdir@/commandline.ggo
	@GENGETOPT@ < @srcdir@/commandline.ggo --output-dir=@srcdir@ --set-version=@VERSION@

EXTRA_DIST= commandline.ggo ccusbReadout_user.xml ccusbReadout_man.xml

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
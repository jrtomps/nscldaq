SUBDIRS = 	vmusb 		\
		devices		\
		router		\
		rdothread	\
		tclserver	\
		daqconfig	\
		 commands	\
		driverkit	\
		tcldrivers	\
		controlpanels	\
		devtests	\
		vmusbPackage \
		tclservertests


bin_PROGRAMS = VMUSBReadout

VMUSBReadout_CXXFLAGS=@THREADCXX_FLAGS@ -I@top_srcdir@/usb/vmusb/vmusb -I@top_srcdir@/usb/vmusb/devices \
	-I@top_srcdir@/usb/vmusb/router \
	-I@top_srcdir@/usb/vmusb/tclserver -I@top_srcdir@/usb/vmusb/daqconfig -I@top_srcdir@/usb/vmusb/commands  \
	-I@top_srcdir@/usb/vmusb/rdothread	\
	-I@top_srcdir@/usb/common/slowcontrols	\
	@LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/thread	\
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/base/dataflow   \
        -I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/os		\
	@TCL_CPPFLAGS@ $(USBSWITCHES) 

VMUSBReadout_CCFLAGS=@THREADC_FLAGS@

VMUSBReadout_SOURCES = CTheApplication.cpp Globals.cpp 

VMUSBReadout_DEPENDENCIES = cmdline.o

BUILT_SOURCES = cmdline.h

noinst_HEADERS = CTheApplication.h Globals.h event.h



VMUSBReadout_LDADD	= cmdline.o @top_builddir@/usb/vmusb/devices/libVMUSBDevices.la	\
		@top_builddir@/usb/vmusb/router/libVMUSBRouter.la	\
		@top_builddir@/usb/vmusb/rdothread/libVMUSBRdoThread.la	\
		@top_builddir@/usb/vmusb/tclserver/libVMUSBControlServer.la	\
		@top_builddir@/usb/vmusb/daqconfig/libVMUSBConfig.la	\
		@top_builddir@/usb/vmusb/commands/libVMUSBCommands.la	\
                @top_builddir@/servers/portmanager/libPortManager.la \
		@top_builddir@/base/dataflow/libDataFlow.la \
		@top_builddir@/usb/vmusb/vmusb/libVMUSB.la	\
		@LIBTCLPLUS_LDFLAGS@	\
		@top_builddir@/base/thread/libdaqthreads.la	\
		@top_builddir@/base/os/libdaqshm.la	\
		$(TCL_LDFLAGS) $(USB_LIBS) @THREADLD_FLAGS@


VMUSBReadout_LDFLAGS=-Wl,"-rpath=$(libdir)"

cmdline.h: commandline.ggo
	$(GENGETOPT) <@top_srcdir@/usb/vmusb/commandline.ggo \
		@output-dir=@top_srcdir@/usb/vmusb --set-version=@VERSION@



cmdline.o: commandline.ggo
	$(GENGETOPT) <@top_srcdir@/usb/vmusb/commandline.ggo --set-version=@VERSION@
	$(CC) -c -I. cmdline.c $(THREADCFLAGS)

EXTRA_DIST=commandline.ggo vmusbReadout_user.xml vmusbReadout_man.xml



SUBDIRS = 	vmusb 		\
		devices		\
		threadcomm	\
		router		\
		rdothread	\
		tclserver	\
		daqconfig	\
		 commands	\
		driverkit	\
		tcldrivers

bin_PROGRAMS = VMUSBReadout

VMUSBReadout_CXXFLAGS=@THREADCXX_FLAGS@
VMUSBReadout_CCFLAGS=@THREADC_FLAGS@

VMUSBReadout_SOURCES = CTheApplication.cpp Globals.cpp 

VMUSBReadout_DEPENDENCIES = cmdline.o

BUILT_SOURCES = cmdline.h

noinst_HEADERS = CTheApplication.h Globals.h

INCLUDES = -I$(top_srcdir)/usb/vmusb/vmusb -I$(top_srcdir)/usb/vmusb/devices -I$(top_srcdir)/usb/vmusb/threadcomm -I$(top_srcdir)/usb/vmusb/router -I$(top_srcdir)/usb/vmusb/rdothreaed \
	-I$(top_srcdir)/usb/vmusb/tclserver -I$(top_srcdir)/usb/vmusb/daqconfig -I$(top_srcdir)/usb/vmusb/commands  \
	-I$(top_srcdir)/base/tclplus -I$(top_srcdir)/base/exception \
	-I$(top_srcdir)/base/thread	\
	-I$(top_srcdir)/base/headers	\
	-I$(top_srcdir)/base/dataflow   \
	$(TCL_FLAGS) $(USBSWITCHES)

VMUSBReadout_LDADD	= cmdline.o $(top_builddir)/usb/vmusb/devices/libVMUSBDevices.la	\
		$(top_builddir)/usb/vmusb/threadcomm/libVMUSBThreadComm.la	\
		$(top_builddir)/usb/vmusb/router/libVMUSBRouter.la	\
		$(top_builddir)/usb/vmusb/rdothread/libVMUSBRdoThread.la	\
		$(top_builddir)/usb/vmusb/tclserver/libVMUSBControlServer.la	\
		$(top_builddir)/usb/vmusb/daqconfig/libVMUSBConfig.la	\
		$(top_builddir)/usb/vmusb/commands/libVMUSBCommands.la	\
		$(top_builddir)/base/dataflow/libDataFlow.la \
		@top_builddir@/usb/vmusb/vmusb/libVMUSB.la	\
		$(top_builddir)/base/tclplus/libtclPlus.la \
		$(top_builddir)/base/exception/libException.la \
		$(top_builddir)/base/thread/libdaqthreads.la	\
		$(TCL_LDFLAGS) $(USB_LIBS) @THREADLD_FLAGS@


VMUSBReadout_LDFLAGS=-Wl,"-rpath=$(libdir)"

cmdline.h: commandline.ggo
	$(GENGETOPT) <commandline.ggo



cmdline.o: commandline.ggo
	$(GENGETOPT) <commandline.ggo
	$(CC) -c -I. cmdline.c $(THREADCFLAGS)

EXTRA_DIST=commandline.ggo

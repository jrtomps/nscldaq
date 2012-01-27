SUBDIRS = 	vmusb 		\
		devices		\
		threadcomm	\
		router		\
		rdothread	\
		tclserver	\
		daqconfig	\
		 commands

bin_PROGRAMS = Readout

Readout_CXXFLAGS=@THREADCXX_FLAGS@
Readout_CCFLAGS=@THREADC_FLAGS@

Readout_SOURCES = CTheApplication.cpp Globals.cpp 

Readout_DEPENDENCIES = cmdline.o

BUILT_SOURCES = cmdline.h

include_HEADERS = CTheApplication.h Globals.h

INCLUDES = -I$(top_srcdir)/usb/vmusb/vmusb -I$(top_srcdir)/usb/vmusb/devices -I$(top_srcdir)/usb/vmusb/threadcomm -I$(top_srcdir)/usb/vmusb/router -I$(top_srcdir)/usb/vmusb/rdothreaed \
	-I$(top_srcdir)/usb/vmusb/tclserver -I$(top_srcdir)/usb/vmusb/daqconfig -I$(top_srcdir)/usb/vmusb/commands  \
	-I$(top_srcdir)/base/tclplus -I$(top_srcdir)/base/exception \
	-I$(top_srcdir)/base/thread	\
	-I$(top_srcdir)/base/headers	\
	$(TCL_FLAGS) $(USBSWITCHES)

Readout_LDADD	= cmdline.o $(top_srcdir)/usb/vmusb/devices/libVMUSBDevices.la	\
		$(top_srcdir)/usb/vmusb/threadcomm/libThreadComm.la	\
		$(top_srcdir)/usb/vmusb/router/libRouter.la	\
		$(top_srcdir)/usb/vmusb/rdothread/libRdoThread.la	\
		$(top_srcdir)/usb/vmusb/tclserver/libControlServer.la	\
		$(top_srcdir)/usb/vmusb/daqconfig/libConfig.la	\
		$(top_srcdir)/usb/vmusb/commands/libCommands.la	\
		$(top_srcdir)/base/tclplus/libtclPlus.la \
		$(top_srcdir)/base/exception/libException.la \
		$(top_srcdir)/base/thread/libdaqthreads.la	\
		$(TCL_LDFLAGS) $(USB_LIBS) @THREADLD_FLAGS@



Readout_LDFLAGS=-Wl,"-rpath=$(libdir)"

cmdline.h: commandline.ggo
	$(GENGETOPT) <commandline.ggo



cmdline.o: commandline.ggo
	$(GENGETOPT) <commandline.ggo
	$(CC) -c -I. cmdline.c $(THREADCFLAGS)

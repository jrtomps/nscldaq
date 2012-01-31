SUBDIRS = 	ccusb 		\
		devices		\
		threadcomm	\
		router		\
		rdothread	\
		daqconfig	\
		tclserver		\
		commands


bin_PROGRAMS = CCUSBReadout

CCUSBReadout_SOURCES = CTheApplication.cpp Globals.cpp 

noinst_HEADERS = CTheApplication.h Globals.h

INCLUDES = -I@top_srcdir@/usb/ccusb/ccusb -I@top_srcdir@/usb/ccusb/devices \
	-I@top_srcdir@/usb/ccusb/threadcomm -I@top_srcdir@/usb/ccusb/router \
	-I@top_srcdir@/usb/ccusb/rdothreaed \
	 -I@top_srcdir@/usb/ccusb/daqconfig -I@top_srcdir@/usb/ccusb/commands \
	-I@top_srcdir@/usb/ccusb/tclserver  \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/tclplus \
	-I@top_srcdir@/base/thread -I@top_srcdir@/base/exception \
	$(TCL_FLAGS)


CCUSBReadout_LDADD	= cmdline.o @top_builddir@/usb/ccusb/devices/libCCUSBDevices.la	\
	@top_builddir@/usb/ccusb/commands/libCommands.la	\
	@top_builddir@/usb/ccusb/daqconfig/libConfig.la		\
	@top_builddir@/usb/ccusb/rdothread/libRdoThread.la	\
	@top_builddir@/usb/ccusb/commands/libCommands.la		\
	@top_builddir@/usb/ccusb/router/libRouter.la		\
	@top_builddir@/usb/ccusb/threadcomm/libThreadComm.la	\
	@top_builddir@/usb/ccusb/ccusb/libCCUSB.la		\
	@top_builddir@/usb/ccusb/tclserver/libControlServer.la \
	@top_builddir@/base/tclplus/libtclPlus.la		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/exception/libException.la		\
	@TCL_LDFLAGS@

CCUSBReadout_LDFLAGS=-Wl,"-rpath=$(libdir)"

CCUSBReadout_DEPENDENCIES = cmdline.o
BUILT_SOURCES = cmdline.h
cmdline.h: commandline.ggo
	$(GENGETOPT) <commandline.ggo



cmdline.o: commandline.ggo
	$(GENGETOPT) <commandline.ggo
	$(CC) -c -I. cmdline.c $(THREADCFLAGS)

EXTRA_DIST=commandline.ggo

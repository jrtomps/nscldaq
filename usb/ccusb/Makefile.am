SUBDIRS = 	ccusb 		\
		devices		\
		threadcomm	\
		router		\
		rdothread	\
		daqconfig	\
		tclserver		\
		 commands


bin_PROGRAMS = Readout

Readout_SOURCES = CTheApplication.cpp Globals.cpp 

include_HEADERS = CTheApplication.h Globals.h config.h

INCLUDES = -Iccusb -Idevices -Ithreadcomm -Irouter -Irdothreaed \
	 -Idaqconfig -Icommands -Itclserver  $(TCL_FLAGS) \
	$(NSCLDAQ_FLAGS)

Readout_LDADD	= -Lccusb -Ldevices -Lthreadcomm -Lrouter -Lrdothread \
	-Ltclserver -Ldaqconfig -Lcommands 				\
	-lCommands -lConfig  -lRdoThread -lRouter -lControlServer -lThreadComm \
	-lVMUSBDevices -lCCUSB						\
	$(NSCLDAQ_LIB) -ltclPlus -lException  $(TCL_LDFLAGS)

Readout_LDFLAGS=-Wl,"-rpath=$(libdir)"

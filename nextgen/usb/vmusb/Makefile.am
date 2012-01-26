SUBDIRS = 	vmusb 		\
		devices		\
		threadcomm	\
		router		\
		rdothread	\
		tclserver	\
		daqconfig	\
		 commands

bin_PROGRAMS = Readout

Readout_SOURCES = CTheApplication.cpp Globals.cpp 

Readout_DEPENDENCIES cmdline.o

BUILT_SOURCES = cmdline.h

include_HEADERS = CTheApplication.h Globals.h config.h

INCLUDES = -Ivmusb -Idevices -Ithreadcomm -Irouter -Irdothreaed \
	-Itclserver -Idaqconfig -Icommands  $(TCL_FLAGS) \
	$(NSCLDAQ_FLAGS)

Readout_LDADD	= -Lvmusb -Ldevices -Lthreadcomm -Lrouter -Lrdothread \
	-Ltclserver -Ldaqconfig -Lcommands 				\
	-lCommands -lConfig -lControlServer -lRdoThread -lRouter -lThreadComm \
	-lVMUSBDevices -lVMUSB						\
	$(NSCLDAQ_LIB) -ltclPlus -lException $(TCL_LDFLAGS) -lrt

Readout_LDFLAGS=-Wl,"-rpath=$(libdir)"

cmdline.h: commandline.ggo
	$(GENGETOPT) <commandline.ggo



cmdline.o: commandline.ggo
	$(GENGETOPT) <commandline.ggo
	$(CC) -c -I. cmdline.c $(THREADCFLAGS)
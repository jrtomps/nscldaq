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

include_HEADERS = CTheApplication.h Globals.h config.h

INCLUDES = -Ivmusb -Idevices -Ithreadcomm -Irouter -Irdothreaed \
	-Itclserver -Idaqconfig -Icommands $(SPECTRODAQ_FLAGS) $(TCL_FLAGS) \
	$(NSCLDAQ_FLAGS)

Readout_LDADD	= -Lvmusb -Ldevices -Lthreadcomm -Lrouter -Lrdothread \
	-Ltclserver -Ldaqconfig -Lcommands 				\
	-lCommands -lConfig -lControlServer -lRdoThread -lRouter -lThreadComm \
	-lVMUSBDevices -lVMUSB						\
	$(NSCLDAQ_LIB) -ltclPlus -lException $(SPECTRODAQ_LDFLAGS) $(TCL_LDFLAGS)
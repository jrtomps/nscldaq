lib_LTLIBRARIES = libReadout.la
libReadout_la_SOURCES = Active.cpp   \
	 CAMACBusy.cpp CESTrigger.cpp  DebugState.cpp \
	EventBufferPointer.cpp Exception.cpp Exiting.cpp Inactive.cpp \
	 Paused.cpp RangeError.cpp Reader.cpp \
	ReadoutStateMachine.cpp RecordingBufferPointer.cpp \
	State.cpp StateMachine.cpp TestBusy.cpp TestTrigger.cpp VMEBusy.cpp \
	VMETrigger.cpp   \
	cmdio.c command.c diagnose.c docmd.c 

include_HEADERS = Active.h Busy.h   CAMACBusy.h \
	 CESTrigger.h CException.h  \
	DebugState.h EventBufferPointer.h Exception.h Exiting.h Inactive.h \
	  Paused.h RangeError.h Reader.h Readout.h \
	ReadoutStateMachine.h RecordingBufferPointer.h  State.h \
	StateMachine.h TestBusy.h TestTrigger.h Transition.h Trigger.h \
	VMEBusy.h VMETrigger.h   \
	cmdio.h daqerrno.h daqinterface.h   \
	skeleton.h  vme.h  

sysconf_DATA = runstate.tran

install-data-local:
	$(mkinstalldirs) $(prefix)/Readout/Skel
	$(mkinstalldirs) $(prefix)/Readout/etc
	rm -f $(prefix)/Readout/Etc
	$(LN_S) $(prefix)/Readout/etc $(prefix)/Readout/Etc
	$(INSTALL_DATA) $(srcdir)/ReadoutMain.cpp \
		$(prefix)/Readout/Skel/ReadoutMain.cpp
	$(INSTALL_DATA) $(srcdir)/skeleton.cpp    \
		$(prefix)/Readout/Skel/skeleton.cpp
	echo BINDIR=$(prefix)/bin                        > Makefile.user
	echo INCLUDEDIR=$(prefix)/include               >> Makefile.user
	echo LIBDIR=$(prefix)/lib                       >> Makefile.user
	echo VMELIBRARY=$(VMELIBRARY)                   >> Makefile.user
	echo VMELIBRARYHEADERS=$(VMELIBRARYHEADERS)     >> Makefile.user
	echo spectrodaq_conf_dir=$(spectrodaq_conf_dir) >> Makefile.user
	echo CAMAC=$(CAMAC)				>> Makefile.user
	cat Makefile.skel                               >> Makefile.user
	$(INSTALL_DATA) Makefile.user $(prefix)/Readout/Skel/Makefile
	$(INSTALL_DATA) runstate.tran $(prefix)/Readout/etc	

libReadout_la_LDFLAGS = -version-info 2:2:0  -L@top_srcdir@/VMEApi \
	-L@top_srcdir@/DeviceSupport -lDeviceSupport
	-lVmeAPI $(SPECTRODAQ_LDFLAGS) -g

INCLUDES = $(VMEBUILDEADERS) \
	-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/DeviceSupport\
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g

EXTRA_DIST = runstate.tran Makefile.skel ReadoutMain.cpp skeleton.cpp 

CXXFLAGS=-g
CCFLAGS=-g

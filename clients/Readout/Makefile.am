lib_LTLIBRARIES = libReadout.la
libReadout_la_SOURCES = Active.cpp CAENcard.cpp  \
	 CAMACBusy.cpp CESTrigger.cpp CaenIO.cpp DebugState.cpp \
	EventBufferPointer.cpp Exception.cpp Exiting.cpp Inactive.cpp \
	MmapError.cpp Nimout.cpp Paused.cpp RangeError.cpp Reader.cpp \
	ReadoutStateMachine.cpp RecordingBufferPointer.cpp \
	State.cpp StateMachine.cpp TestBusy.cpp TestTrigger.cpp VMEBusy.cpp \
	VMETrigger.cpp VmeModule.cpp camacmap.cpp sis3300.cpp \
	cmdio.c command.c diagnose.c docmd.c CAENcard_767.cpp v890.cpp \
	CCAENChain.cpp


include_HEADERS = Active.h Busy.h CAENcard.h CCAENChain.h CAMACBusy.h \
	CCaenIO.h CESTrigger.h CException.h CVME.h CVMEptr.h CaenIO.h \
	DebugState.h EventBufferPointer.h Exception.h Exiting.h Inactive.h \
	MmapError.h Nimout.h Paused.h RangeError.h Reader.h Readout.h \
	ReadoutStateMachine.h RecordingBufferPointer.h  State.h \
	StateMachine.h TestBusy.h TestTrigger.h Transition.h Trigger.h \
	VMEBusy.h VMETrigger.h VmeModule.h  \
	cmdio.h daqerrno.h daqinterface.h  CAENcard_767.h  \
	 sis3300.h \
	skeleton.h  vme.h cesmacros.h wienermacros.h macros.h \
	cescamac.h wienercamac.h camac.h v890.h


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

libReadout_la_LDFLAGS = -version-info 2:1:0  -L@top_srcdir@/VMEApi \
	-lVmeAPI $(SPECTRODAQ_LDFLAGS) -g

INCLUDES = $(VMELIBRARYHEADERS) \
	-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi \
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g

EXTRA_DIST = runstate.tran Makefile.skel ReadoutMain.cpp skeleton.cpp 

CXXFLAGS=-g
CCFLAGS=-g

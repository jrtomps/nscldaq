lib_LTLIBRARIES = libReadout.la
libReadout_la_SOURCES = Active.cpp CAENcard.cpp CAENcard_test.cpp \
	CAENchain.cpp CAMACBusy.cpp CESTrigger.cpp CaenIO.cpp DebugState.cpp \
	EventBufferPointer.cpp Exception.cpp Exiting.cpp Inactive.cpp \
	MmapError.cpp Nimout.cpp Paused.cpp RangeError.cpp Reader.cpp \
	ReadoutStateMachine.cpp RecordingBufferPointer.cpp \
	State.cpp StateMachine.cpp TestBusy.cpp TestTrigger.cpp VMEBusy.cpp \
	VMETrigger.cpp VmeModule.cpp camacmap.cpp sis3300.cpp \
	cmdio.c command.c diagnose.c docmd.c CAENcard_767.cpp


include_HEADERS = Active.h Busy.h CAENcard.h CAENchain.h CAMACBusy.h \
	CCaenIO.h CESTrigger.h CException.h CVME.h CVMEptr.h CaenIO.h \
	DebugState.h EventBufferPointer.h Exception.h Exiting.h Inactive.h \
	MmapError.h Nimout.h Paused.h RangeError.h Reader.h Readout.h \
	ReadoutStateMachine.h RecordingBufferPointer.h  State.h \
	StateMachine.h TestBusy.h TestTrigger.h Transition.h Trigger.h \
	VMEBusy.h VMETrigger.h VmeModule.h  \
	cmdio.h daqerrno.h daqinterface.h  CAENcard_767.h  \
	 sis3300.h \
	skeleton.h  vme.h cesmacros.h wienermacros.h \
	cescamac.h wienercamac.h camac.h


sysconf_DATA = runstate.tran

install-data-local:
	$(mkinstalldirs) $(prefix)/Readout/Skel
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
	

libReadout_la_LDFLAGS = -version-info 1:0:0  -L@top_srcdir@/VMEApi \
	-lVmeApi $(SPECTRODAQ_LDFLAGS)

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/VMEApi \
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC)

EXTRA_DIST = runstate.tran Makefile.skel ReadoutMain.cpp skeleton.cpp 



lib_LTLIBRARIES = libHPReadout.la
libHPReadout_la_SOURCES = Active.cpp   \
	 CAMACBusy.cpp CESTrigger.cpp  DebugState.cpp \
	EventBufferPointer.cpp Exception.cpp Exiting.cpp Inactive.cpp \
	 Paused.cpp Reader.cpp \
	ReadoutStateMachine.cpp RecordingBufferPointer.cpp \
	State.cpp StateMachine.cpp TestBusy.cpp TestTrigger.cpp VMEBusy.cpp \
	VMETrigger.cpp   \
	cmdio.c command.c diagnose.c docmd.c \
	CTraditionalV977Busy.cpp CTraditionalV977Trigger.cpp

include_HEADERS = Active.h Busy.h   CAMACBusy.h \
	 CESTrigger.h CException.h  \
	DebugState.h EventBufferPointer.h Exception.h Exiting.h Inactive.h \
	  Paused.h  Reader.h Readout.h \
	ReadoutStateMachine.h RecordingBufferPointer.h  State.h \
	StateMachine.h TestBusy.h TestTrigger.h Transition.h Trigger.h \
	VMEBusy.h VMETrigger.h   \
	cmdio.h daqerrno.h daqinterface.h   \
	skeleton.h  vme.h  \
	CTraditionalV977Busy.h CTraditionalV977Trigger.h

sysconf_DATA = runstate.tran

install-data-local:
	$(mkinstalldirs) $(prefix)/Readout/hpskel
	$(mkinstalldirs) $(prefix)/Readout/hpetc
	$(mkinstalldirs) $(prefix)/etc
	rm -f $(prefix)/Readout/HPEtc
	$(LN_S) $(prefix)/Readout/etc $(prefix)/Readout/HPEtc
	$(INSTALL_DATA) $(srcdir)/ReadoutMain.cpp \
		$(prefix)/Readout/hpskel/ReadoutMain.cpp
	$(INSTALL_DATA) $(srcdir)/skeleton.cpp    \
		$(prefix)/Readout/hpskel/skeleton.cpp
	echo INSTDIR=$(prefix)                           > Makefile.user
	cat Makefile.skel                               >> Makefile.user
	$(INSTALL_DATA) $(srcdir)/skeleton.h $(prefix)/Readout/hpskel
	$(INSTALL_DATA) Makefile.user $(prefix)/Readout/hpskel/Makefile
	$(INSTALL_DATA) Readout_Makefile.include $(prefix)/etc/HPReadout_Makefile.include
	$(INSTALL_DATA) runstate.tran $(prefix)/Readout/hpetc	

libHPReadout_la_LIBADD = @top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
			@top_srcdir@/DeviceSupport/libDeviceSupport.la	\
			 $(VMEBUILDLIBS) \
			 $(SPECTRODAQ_LDFLAGS)


libHPReadout_la_LDFLAGS = -version-info $(SOVERSION):0 \
			-g -Wl,"-rpath-link=$(libdir)"

INCLUDES = $(VMEBUILDEADERS) \
	-I@top_srcdir@/SpectrodaqLite			\
	-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/DeviceSupport\
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g

EXTRA_DIST = runstate.tran Makefile.skel ReadoutMain.cpp skeleton.cpp  \
	Readout_Makefile.include

CXXFLAGS=-g
CCFLAGS=-g

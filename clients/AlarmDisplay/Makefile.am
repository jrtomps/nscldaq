#
#  Need to build and install both the alarm server and the
#  scripts associated with it.
#

# Alarm server application
bin_PROGRAMS = AlarmServerApp
AlarmServerApp_SOURCES = AlarmServerApp.cpp CAlarmServer.cpp \
	CAlarmServerException.cpp CGDBMException.cpp
INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/framework -I@top_srcdir@/TCL $(SPECTRODAQ_FLAGS) \
	$(TCL_FLAGS) $(X11INCS)
AlarmServerApp_LDADD= -L@top_srcdir@/framework -L@top_srcdir@/Exception \
	-L@top_srcdir@/TCL -ltclPlus \
	-lEventFramework -lException $(SPECTRODAQ_LDFLAGS)  \
	$(TCL_LDFLAGS) $(X11LIBS) -lm 
AlarmServerApp_LDFLAGS=-Wl,"-rpath=$(libdir)"
#
#  Scripts (and their associated gifs:
#
EXTRA_DIST= CAlarmServer.h CAlarmServerException.h CGDBMException.h \
	AckButton.gif \
	AlarmDisplay.tcl \
	CancelButton.gif \
	DisButton.gif \
	ExitButton.gif \
	HistButton.gif \
	OkButton.gif \
	RemButton.gif \
	RetryButton.gif \
	installpkg

TCLAPPS=AlarmDisplay.tcl

install-exec-local:
	$(mkinstalldirs) $(prefix)/TclLibs/data/AlarmDisplay
	for f in *.gif; do $(INSTALL_DATA) $$f $(prefix)/TclLibs/data/AlarmDisplay; done
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) $$f  $(prefix)/bin/`basename $$f .tcl`; done

noinst_HEADERS = CAlarmServerException.h
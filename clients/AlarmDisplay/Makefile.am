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
	-L@top_srcdir@/TCL -lTclPlus \
	-lEventFramework -lException $(SPECTRODAQ_LDFLAGS)  \
	$(TCL_LDFLAGS) $(X11LIBS) -lm
#
#  Scripts (and their associated gifs:
#
EXTRA_DIST= CAlarmServer.h CAlarmServerException.h CGDBMException.h \
	AlarmDisplay \
	AckButton.gif \
	AlarmDisplay.tcl \
	CancelButton.gif \
	DisButton.gif \
	ExitButton.gif \
	HistButton.gif \
	OkButton.gif \
	RemButton.gif \
	RetryButton.gif \
	mwutil.tcl \
	tablelist.tcl \
	tablelistSortByColumn.tcl \
	tablelistWidget.tcl \
	installpkg
	
install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(prefix)/Images
	for f in *.gif; do $(INSTALL_DATA) $$f $(prefix)/Images; done
	for f in *.tcl; do $(INSTALL_DATA) $$f $(prefix)/Scripts; done
	$(INSTALL_SCRIPT) AlarmDisplay $(bindir)
	./installpkg $(prefix)

noinst_HEADERS = CAlarmServerException.h
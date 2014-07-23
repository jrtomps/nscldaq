PACKAGE_FILES = StateMachine.py GenericState.py  Utilities.py StateMonitor.py

MAINS = stateManager.py

INSTDIR = @prefix@/pythonLibs/nscldaq/statemanager
TCLLIBDIR=@prefix@/TclLibs

FIGURES=statediagram.jpg

##
# installation hook: create the installation directory tree.
#                    install the packges in  INSTDIR
#                    ensure there's an __init__.py which is required for
#                    package directories.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	$(mkinstalldirs)  @datarootdir@/html
	touch $(INSTDIR)/__init__.py 
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done
	$(mkinstalldirs) $(TCLLIBDIR)/statemanager
	echo package ifneeded statemanager 1.0 [list load [file join @libdir@ libStateMonitor.so]] > $(TCLLIBDIR)/statemanager/pkgIndex.tcl
	for f in $(FIGURES); do $(INSTALL_DATA) @srcdir@/$$f @datarootdir@/html; done



EXTRA_DIST=$(PACKAGE_FILES) $(MAINS) $(FIGURES) \
	MonitorTest.py CommandTest.py StateMonitorTest1.py tcltest1.tcl statemanager.xml


## 
# Bindings for C++


INCLUDES  = @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@


lib_LTLIBRARIES = libStateMonitor.la

libStateMonitor_la_SOURCES=CStateMonitor.cpp CTCLStateMonitor.cpp
libStateMonitor_ladir     = @includedir@
libStateMonitor_la_HEADERS= CStateMonitor.h CTCLStateMonitor.h 
libStateMonitor_la_LIBADD = @LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@ @ZMQ_LDFLAGS@

noinst_PROGRAMS=Polltest StateMonitorBaseTest StateMonitorTest tclTest1
Polltest_SOURCES=Polltest.cpp
Polltest_LDFLAGS=libStateMonitor.la @ZMQ_LDFLAGS@

StateMonitorBaseTest_SOURCES=CStateMonitorBaseTest.cpp
StateMonitorBaseTest_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@

StateMonitorTest_SOURCES=CStateMonitorTest.cpp
StateMonitorTest_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@

tclTest1_SOURCES=tclTest1.cpp
tclTest1_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ @ZMQ_LDFLAGS@ @TCL_LIBS@ -Wl,"-rpath=@libdir@"


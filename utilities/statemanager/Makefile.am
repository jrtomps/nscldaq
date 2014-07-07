PACKAGE_FILES = StateMachine.py GenericState.py  Utilities.py StateMonitor.py

MAINS = stateManager.py

INSTDIR = @prefix@/pythonLibs/nscldaq/statemanager
TCLLIBDIR=@prefix@/TclLibs

##
# installation hook: create the installation directory tree.
#                    install the packges in  INSTDIR
#                    ensure there's an __init__.py which is required for
#                    package directories.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	touch $(INSTDIR)/__init__.py 
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done
	$(mkinstalldirs) $(TCLLIBDIR)/statemanager
	echo package ifneeded statemanager 1.0 [list load [file join @libdir@ libStateMonitor.so]] > $(TCLLIBDIR)/statemanager/pkgIndex.tcl


EXTRA_DIST=$(PACKAGE_FILES) $(MAINS) \
	MonitorTest.py CommandTest.py StateMonitorTest1.py tcltest1.tcl


## 
# Bindings for C++


INCLUDES  = @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@


lib_LTLIBRARIES = libStateMonitor.la

libStateMonitor_la_SOURCES=CStateMonitor.cpp CTCLStateMonitor.cpp
libStateMonitor_ladir     = @includedir@
libStateMonitor_la_HEADERS= CStateMonitor.h CTCLStateMonitor.h 
libStateMonitor_la_LIBADD = @LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@ -lzmq

noinst_PROGRAMS=Polltest StateMonitorBaseTest StateMonitorTest tclTest1
Polltest_SOURCES=Polltest.cpp
Polltest_LDFLAGS=libStateMonitor.la -lzmq 

StateMonitorBaseTest_SOURCES=CStateMonitorBaseTest.cpp
StateMonitorBaseTest_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ -lzmq

StateMonitorTest_SOURCES=CStateMonitorTest.cpp
StateMonitorTest_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ -lzmq

tclTest1_SOURCES=tclTest1.cpp
tclTest1_LDFLAGS=libStateMonitor.la @LIBTCLPLUS_LDFLAGS@ -lzmq @TCL_LIBS@ -Wl,"-rpath=@libdir@"


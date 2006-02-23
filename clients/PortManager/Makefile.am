#
#   Build the C++ library:
#
lib_LTLIBRARIES=libPortManager.la
libPortManager_la_SOURCES = CPortManagerException.cpp \
						    CPortManager.cpp

include_HEADERS = CPortManagerException.h \
				  CPortManager.h
libPortManager_la_LDFLAGS = -L@top_srcdir@/Exception \
			-lException   \
			-version-info $(SOVERSION):0 \
			-Wl,"-rpath=$(libdir)"

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
		-DHOME=\"$(prefix)\"

TCLPACKAGES=ConnectionManager.tcl \
	Log.tcl \
	portAllocator.tcl  \
	PortManager.tcl

TCLAPPS=DaqPortManager.tcl

TCLTESTS=connectionmanager.test portallocator.test portmanager.test log.test


#
#   Install the TCL Packages:
#   We're counting on the top level makefile to build the package directory.
#
install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/PortManager
	$(INSTALL_SCRIPT) $(TCLPACKAGES) $(prefix)/TclLibs/PortManager
	$(INSTALL_SCRIPT) $(TCLAPPS)     $(prefix)/bin/DaqPortManager
	echo "pkg_mkIndex $(prefix)/TclLibs/PortManager *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

#
#  The distribution should contain the tcl files as well as their unit tests:
#
EXTRA_DIST =$(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS)          \
	DaqPortManager.8 portAllocator.3tcl CPortManager.3 \
	CPortManagerException.3
#
#  The tests are all TCL tests and can be unconditionally run.
#  We rely on the value of TCLSH_CMD from configure to know how to run the
#  shell.  TCLSH_CMD must represent a tcl shell of at least version 8.4.
#
noinst_PROGRAMS = cpptest

cpptest_SOURCES = cpptest.cpp
cpptest_LDADD= -L. -lPortManager -L@top_srcdir@/Exception -lException

check-TESTS: cpptest
	unittests $(TCLSH_CMD)
	cpptest
man_MANS = DaqPortManager.8 portAllocator.3tcl CPortManager.3 CPortManagerException.3
#
#   Build the C++ library:
#
lib_LTLIBRARIES=libPortManager.la
libPortManager_la_SOURCES = CPortManagerException.cpp \
			    CPortManager.cpp

include_HEADERS = CPortManagerException.h \
		  CPortManager.h

libPortManager_la_LDFLAGS = @LIBEXCEPTION_LDFLAGS@	\
			@top_builddir@/base/os/libdaqshm.la   \
			-version-info $(SOVERSION) \
			-Wl,"-rpath=$(libdir)"	\
			$(TCL_LDFLAGS)		\
			$(THREADLD_FLAGS)

libPortManager_la_CXXFLAGS = $(THREADCXX_FLAGS) 	\
				$(TCL_FLAGS)		\
				$(AM_CXXFLAGS)

INCLUDES = -I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/os		\
		-DHOME=\"@prefix@\"

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
	$(mkinstalldirs) @prefix@/bin
	$(mkinstalldirs) @prefix@/TclLibs/PortManager
	for f in  $(TCLPACKAGES); do @INSTALL_SCRIPT@ @srcdir@/$$f @prefix@/TclLibs/PortManager; done
	for f in  $(TCLAPPS); do @INSTALL_SCRIPT@ @srcdir@/$$f     @prefix@/bin/DaqPortManager; done
	echo "pkg_mkIndex @prefix@/TclLibs/PortManager *.tcl" | @TCLSH_CMD@


#
#  The distribution should contain the tcl files as well as their unit tests:
#
EXTRA_DIST =$(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS)  portmanager.xml

#
#  The tests are all TCL tests and can be unconditionally run.
#  We rely on the value of TCLSH_CMD from configure to know how to run the
#  shell.  TCLSH_CMD must represent a tcl shell of at least version 8.4.
#
noinst_PROGRAMS = cpptest

cpptest_SOURCES = cpptest.cpp
cpptest_LDADD= -L. -lPortManager @LIBEXCEPTION_LDFLAGS@

check-TESTS: cpptest
	for f in @srcdir@/*.test;do @TCLSH_CMD@ $$f; done
	cpptest

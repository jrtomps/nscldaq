lib_LTLIBRARIES = libServices.la libTclServices.la libPyServices.la

TCL_APPS=addsysprogram rmsysprogram modsysprogram lsservices svcpropset

libServices_la_SOURCES = CServiceApi.cpp
libServices_la_CPPFLAGS= @SQLITE3_CFLAGS@ -I@top_srcdir@/base/factories \
	-I@top_srcdir@/utilities/varmanager -I@top_srcdir@/utilities/varmanager/server \
	-I@top_srcdir@/base/sqlite++
libServices_la_LDFLAGS= @top_builddir@/utilities/varmanager/server/libvarmgr.la \
	@top_builddir@/utilities/varmanager/libvardb.la \
	@top_builddir@/base/sqlite++/libsqlite3pp.la 	\
	@SQLITE3_LDFLAGS@


libTclServices_la_SOURCES = TclServicesPackage.cpp \
	CTCLServiceApiCommand.cpp CTCLServiceApiCommand.h \
	CTCLServiceApiInstance.cpp CTCLServiceApiInstance.h
libTclServices_la_CPPFLAGS=$(libServices_la_CPPFLAGS) @TCL_CPPFLAGS@ @LIBTCLPLUS_CFLAGS@
libTclServices_la_LDFLAGS=libServices.la $(libServices_la_LDFLAGS) \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@

libPyServices_la_SOURCES=pyServices.cpp
libPyServices_la_CPPFLAGS=@PYTHON_CPPFLAGS@ $(libServices_la_CPPFLAGS)
libPyServices_la_LDFLAGS=@PYTHON_LDFLAGS@ libServices.la $(libServices_la_LDLFAGS) 


include_HEADERS = CServiceApi.h


noinst_PROGRAMS=unittest
unittest_SOURCES=TestRunner.cpp apiTests.cpp   \
	$(libServices_la_SOURCES)

unittest_CPPFLAGS=$(libServices_la_CPPFLAGS) @CPPUNIT_CFLAGS@
unittest_LDFLAGS =$(libServices_la_LDFLAGS) @CPPUNIT_LDFLAGS@

EXTRA_DIST=TclServices.test apiTests.py services.xml ServiceWizard.py \
	addsysprogram.in lsservices.in modsysprogram.in rmsysprogram.in \
	svcwiz.in svcpropset.in svcmanager.py

EST_EXTENSIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH; \
	DAQBIN=@prefix@/bin; export DAQBIN; DAQROOT=@prefix@; export DAQROOT; \
	DAQLIB=@prefix@/lib; export DAQLIB;

TESTS=unittest TclServices.test apiTests.py



install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs/services
	rm -f @prefix@/TclLibs/services/libTclServices.so @prefix@/TclLibs/services/pkgIndex.tcl
	$(INSTALL_PROGRAM) @prefix@/lib/libTclServices.so  @prefix@/TclLibs/services
	echo package ifneeded daqservices 1.0 [list load [file join \$$dir libTclServices.so]] > @prefix@/TclLibs/services/pkgIndex.tcl
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	$(INSTALL_PROGRAM) @libdir@/libPyServices.so @prefix@/pythonLibs/nscldaq/vardb/services.so
	for f in $(TCL_APPS) ; do $(INSTALL_PROGRAM) $$f @bindir@; done
	$(INSTALL_PROGRAM) svcwiz @bindir@
	$(INSTALL_PROGRAM) @srcdir@/ServiceWizard.py @bindir@
	$(INSTALL_PROGRAM) @srcdir@/svcmanager.py @bindir@/svcmanager.py
	echo "#!/bin/bash"    > @bindir@/svcmanager
	echo @PYTHON@ @prefix@/bin/svcmanager.py \$$1 >>@bindir@/svcmanager
	chmod a+x @bindir@/svcmanager


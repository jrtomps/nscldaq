lib_LTLIBRARIES=libvarmgr.la

COMMONLDFLAGS = @builddir@/../libvardb.la 	\
		@top_builddir@/base/sqlite++/libsqlite3pp.la 	\
		@SQLITE3_LDFLAGS@ @ZMQ_LDFLAGS@

libvarmgr_la_SOURCES = CVarMgrApi.cpp CVarMgrFileApi.cpp CVarMgrServerApi.cpp
libvarmgr_la_CPPFLAGS=-I@srcdir@/.. @SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqlite++
libvarmgr_la_LDFLAGS = $(COMMONLDFLAGS)

include_HEADERS=CVarMgrApi.h CVarMgrFileApi.h CVarMgrServerApi.h


install-exec-local:
	echo "#!@PYTHON@" >  @bindir@/vardbServer
	cat @srcdir@/vardbServer.py >>@bindir@/vardbServer
	chmod a+x @bindir@/vardbServer


SERVER_TESTS=startTests.py serviceTests.py mkdirTests.py  \
	testGet.py

noinst_PROGRAMS = testApi

testApi_SOURCES=TestRunner.cpp TestApiBase.cpp testDirect.cpp \
	testServer.cpp \
	$(libvarmgr_la_SOURCES)
testApi_CPPFLAGS=-DBINDIR=\"@bindir@\" \
	-I@top_srcdir@/servers/portmanager -I@top_srcdir@/base/os \
	 @LIBTCLPLUS_CFLAGS@ \
	@CPPUNIT_CFLAGS@ @SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqlite++ -I..
testApi_LDFLAGS = $(COMMONLDFLAGS) \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@	\
	 @CPPUNIT_LDFLAGS@

check-TESTS:
	for f in $(SERVER_TESTS); do echo $$f : ; PYTHONPATH=@prefix@/pythonLibs  BINDIR=@bindir@  @PYTHON@ @srcdir@/$$f; done
	testApi


EXTRA_DIST=$(SERVER_TESTS) testBase.py vardbServer.py varserver.xml


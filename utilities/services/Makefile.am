lib_LTLIBRARIES = libServices.la

libServices_la_SOURCES = CServiceApi.cpp
libServices_la_CPPFLAGS= @SQLITE3_CFLAGS@ -I@top_srcdir@/base/factories \
	-I@top_srcdir@/utilities/varmanager -I@top_srcdir@/utilities/varmanager/server \
	-I@top_srcdir@/base/sqlite++
libServices_la_LDFLAGS= @top_builddir@/utilities/varmanager/server/libvarmgr.la \
	@top_builddir@/utilities/varmanager/libvardb.la \
	@top_builddir@/base/sqlite++/libsqlite3pp.la 	\
	@SQLITE3_LDFLAGS@

include_HEADERS = CServiceApi.h


noinst_PROGRAMS=unittest
unittest_SOURCES=TestRunner.cpp apiTests.cpp   \
	$(libServices_la_SOURCES)
	
unittest_CPPFLAGS=$(libServices_la_CPPFLAGS) @CPPUNIT_CFLAGS@
unittest_LDFLAGS =$(libServices_la_LDFLAGS) @CPPUNIT_LDFLAGS@

check-TESTS:
	@builddir@/unittest
	
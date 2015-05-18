#
# Primary targets are the APIS used by clients and managers:

lib_LTLIBRARIES = libstateclient.la

#  Common flag definitions so even the Makefile.am is DRY:

COMMON_CPPFLAGS=-I@top_srcdir@/utilities/varmanager/server -I@top_srcdir@/base/thread \
		-I@top_srcdir@/base/os -I@top_srcdir@/base/uri \
		@LIBTCLPLUS_CFLAGS@ @GLIB_CFLAGS@
COMMON_LOADFLAGS= @top_builddir@/utilities/varmanager/server/libvarmgr.la \
		@top_builddir@/utilities/varmanager/libvardb.la \
		@top_builddir@/servers/portmanager/libPortManager.la \
		@top_builddir@/base/thread/libdaqthreads.la      \
		@top_builddir@/base/os/libdaqshm.la		\
		@top_builddir@/base/sqlite++/libsqlite3pp.la  @SQLITE3_LDFLAGS@  \
		@LIBEXCEPTION_LDFLAGS@ @GLIB_LIBS@


#

libstateclient_la_SOURCES=CStateClientApi.cpp CStateChangeMonitorThread.cpp CStateChangeMonitorThread.h
libstateclient_la_CPPFLAGS=$(COMMON_CPPFLAGS)
libstateclient_la_LDFALGS=$(COMMON_LOADFLAGS)


# Headers for public APIS:

include_HEADERS = CStateClientApi.h



#---------------------------------------------------------------------------------
# tests:
#


noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp Asserst.h TestStateChangeMonitor.cpp

unittests_CPPFLAGS=$(COMMON_CPPFLAGS) $(CPPUNIT_CFLAGS) -DBINDIR=\"@bindir@\"
unittests_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) $(CPPUNIT_LDFLAGS)


check-TESTS:
	BINDIR="@bindir@" PYTHONPATH=@prefix@/pythonLibs @builddir@/unittests
lib_LTLIBRARIES = libvardbEventBuilder.la

COMMONLDFLAGS = @top_builddir@/utilities/varmanager/server/libvarmgr.la \
		@top_builddir@/utilities/varmanager/libvardb.la         \
		@top_builddir@/base/sqlite++/libsqlite3pp.la @SQLITE3_LDFLAGS@ \
		@ZMQ_LDFLAGS@							\
		-Wl,"-rpath=@libdir@"
COMMONCPPFLAGS= -I@top_srcdir@/utilities/varmanager/server 	\
	-I@top_srcdir@/utilities/varmanager			\
	-I@top_srcdir@/base/sqlite++

libvardbEventBuilder_la_SOURCES = CVardbEventBuilder.cpp
libvardbEventBuilder_la_CPPFLAGS=$(COMMONCPPFLAGS)
libvardbEventBuilder_la_LDFLAGS=$(COMMONLDFLAGS)

include_HEADERS =  CVardbEventBuilder.h

noinst_PROGRAMS=unittests
noinst_HEADERS=Asserts.h

unittests_SOURCES=TestRunner.cpp apitests.cpp
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS=libvardbEventBuilder.la $(COMMONLDFLAGS) @CPPUNIT_LDFLAGS@


TESTS=@builddir@/unittests
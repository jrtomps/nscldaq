lib_LTLIBRARIES=libvardbRingBuffer.la

COMMONLDFLAGS = @top_builddir@/utilities/varmanager/server/libvarmgr.la \
		@top_builddir@/utilities/varmanager/libvardb.la         \
		@top_builddir@/base/sqlite++/libsqlite3pp.la @SQLITE3_LDFLAGS@ \
		@ZMQ_LDFLAGS@							\
		-Wl,"-rpath=@libdir@"
COMMONCPPFLAGS= -I@top_srcdir@/utilities/varmanager/server 	\
	-I@top_srcdir@/utilities/varmanager			\
	-I@top_srcdir@/base/sqlite++

#  C++ API:

libvardbRingBuffer_la_SOURCES = CVardbRingBuffer.cpp
libvardbRingBuffer_la_CPPFLAGS=$(COMMONCPPLFAGS)
libvardbRingBuffer_la_LDFLAGS=$(COMMONLDFLAGS)

# C++ API tests:

noinst_PROGRAMS=unittests

unittests_SOURCES = TestRunner.cpp Asserts.h
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFDLAGS= libvardbRingBuffer.la $(COMMONLDFLAGS) @CPPUNIT_LDFLAGS@

TESTS=@builddir@/unittests
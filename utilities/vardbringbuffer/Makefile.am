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
libvardbRingBuffer_la_CPPFLAGS=$(COMMONCPPFLAGS)
libvardbRingBuffer_la_LDFLAGS=$(COMMONLDFLAGS)

include_HEADERS=CVardbRingBuffer.h

# C++ API tests:

noinst_PROGRAMS=unittests

unittests_SOURCES = TestRunner.cpp Asserts.h apitests.cpp
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS= libvardbRingBuffer.la $(COMMONLDFLAGS) @CPPUNIT_LDFLAGS@

TESTS=@builddir@/unittests

# Documentation

DOCFILES=man3.xml man5.xml

# Extra files in the distro tarball


EXTRA_DIST=$(DOCFILES)
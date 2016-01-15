lib_LTLIBRARIES=libvardbRingBuffer.la libtclvardbringbuffer.la

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

# TCL API:

libtclvardbringbuffer_la_SOURCES = TCLVardbRingBufferPackage.cpp \
	CTCLVardbRingBuffer.cpp CTCLVardbRingBuffer.h	\
	CTCLVardbRingBufferInstance.cpp CTCLVardbRingBufferInstance.h

libtclvardbringbuffer_la_CPPFLAGS = $(COMMONCPPFLAGS) @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@
libtclvardbringbuffer_la_LDFLAGS = $(COMMONLDFLAGS) @builddir@/libvardbRingBuffer.la \
	@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@

# Installation automake doesn't know how to do:

install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs/vardbringbuffer
	rm -f @prefix@/TclLibs/vardbringbuffer/libtclvardbringbuffer.so
	ln -s @libdir@/libtclvardbringbuffer.so \
		@prefix@/TclLibs/vardbringbuffer/libtclvardbringbuffer.so
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/vardbringbuffer *.so" | @TCLSH@

# C++ API tests:

noinst_PROGRAMS=unittests

unittests_SOURCES = TestRunner.cpp Asserts.h apitests.cpp
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS= libvardbRingBuffer.la $(COMMONLDFLAGS) @CPPUNIT_LDFLAGS@

# Testing setup for TCL and Python tests:

TEST_EXENSTIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPAH=@prefix@/pythonLibs; export PYTHONPATH;

TESTFILES=tclapitest.test

TESTS=@builddir@/unittests $(TESTFILES)

# Documentation

DOCFILES=man3.xml man5.xml

# Extra files in the distro tarball


EXTRA_DIST=$(DOCFILES)  $(TESTFILES)
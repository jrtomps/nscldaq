lib_LTLIBRARIES = libvardbEventBuilder.la libTclvardbEventBuilder.la

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

libTclvardbEventBuilder_la_SOURCES=CTCLVarDbEventBuilder.h CTCLVarDbEventBuilder.cpp \
	CTCLEvbInstance.cpp CTCLEvbInstance.h						\
	EventBuilderTclPkgInit.cpp
libTclvardbEventBuilder_la_CPPFLAGS=$(COMMONCPPFLAGS) @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@
libTclvardbEventBuilder_la_LDFLAGS=libvardbEventBuilder.la $(COMMONLDFLAGS) @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@


install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs/vardbevb
	rm -f @prefix@/TclLibs/vardbevb/libTclvardbEventBuilder.so
	ln -s @libdir@/libTclvardbEventBuilder.so @prefix@/TclLibs/vardbevb/libTclvardbEventBuilder.so
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/vardbevb *.so" | @TCLSH@

include_HEADERS =  CVardbEventBuilder.h

noinst_PROGRAMS=unittests
noinst_HEADERS=Asserts.h

unittests_SOURCES=TestRunner.cpp apitests.cpp
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS=libvardbEventBuilder.la $(COMMONLDFLAGS)   @CPPUNIT_LDFLAGS@


TEST_EXENSTIONS=.tcl
TEST_LOG_COMPILER=@TCLSH@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; 

TESTS=@builddir@/unittests tclpkgtests.test


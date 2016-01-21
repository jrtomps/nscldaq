lib_LTLIBRARIES = libvardbEventBuilder.la libTclvardbEventBuilder.la \
	libPyVarDbEvb.la

COMMONLDFLAGS = @top_builddir@/utilities/varmanager/server/libvarmgr.la \
		@top_builddir@/utilities/varmanager/libvardb.la         \
		@top_builddir@/base/sqlite++/libsqlite3pp.la @SQLITE3_LDFLAGS@ \
		@ZMQ_LDFLAGS@							\
		-Wl,"-rpath=@libdir@"
COMMONCPPFLAGS= -I@top_srcdir@/utilities/varmanager/server 	\
	-I@top_srcdir@/utilities/varmanager			\
	-I@top_srcdir@/base/sqlite++

# C++ API

libvardbEventBuilder_la_SOURCES = CVardbEventBuilder.cpp
libvardbEventBuilder_la_CPPFLAGS=$(COMMONCPPFLAGS)
libvardbEventBuilder_la_LDFLAGS=$(COMMONLDFLAGS)

# TCL Bindings

libTclvardbEventBuilder_la_SOURCES=CTCLVarDbEventBuilder.h CTCLVarDbEventBuilder.cpp \
	CTCLEvbInstance.cpp CTCLEvbInstance.h						\
	EventBuilderTclPkgInit.cpp
libTclvardbEventBuilder_la_CPPFLAGS=$(COMMONCPPFLAGS) @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@
libTclvardbEventBuilder_la_LDFLAGS=libvardbEventBuilder.la $(COMMONLDFLAGS) @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@

#PYTHON Bindings

libPyVarDbEvb_la_SOURCES=PyEventBuilder.cpp
libPyVarDbEvb_la_CPPFLAGS=$(COMMONCPPFLAGS) @PYTHON_CPPFLAGS@
libPyVarDbEvb_la_LDFLAGS=libvardbEventBuilder.la $(COMMONLDFLAGS) @PYTHON_LDFLAGS@


install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs/vardbevb
	rm -f @prefix@/TclLibs/vardbevb/libTclvardbEventBuilder.so
	ln -s @libdir@/libTclvardbEventBuilder.so @prefix@/TclLibs/vardbevb/libTclvardbEventBuilder.so
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/vardbevb *.so" | @TCLSH@
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	rm -f @prefix@/pythonLibs/nscldaq/vardb/VardbEvb.so
	ln -s @libdir@/libPyVarDbEvb.so @prefix@/pythonLibs/nscldaq/vardb/VardbEvb.so

include_HEADERS =  CVardbEventBuilder.h

noinst_PROGRAMS=unittests
noinst_HEADERS=Asserts.h

unittests_SOURCES=TestRunner.cpp apitests.cpp
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS=libvardbEventBuilder.la $(COMMONLDFLAGS)   @CPPUNIT_LDFLAGS@


TEST_EXTENSIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH;

TESTS=@builddir@/unittests tclpkgtests.test pyevbtests.py pydstests.py

EXTRA_DIST=tclpkgtests.test pyevbtests.py pydstests.py 5evb.xml 3evb.xml


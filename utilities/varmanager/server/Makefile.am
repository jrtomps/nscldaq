lib_LTLIBRARIES=libvarmgr.la libvarmgrtcl.la libvarmgrpy.la

COMMONLDFLAGS = @builddir@/../libvardb.la 	\
		@top_builddir@/base/sqlite++/libsqlite3pp.la 	\
		@top_builddir@/base/uri/liburl.la		\
		@top_builddir@/base/os/libdaqshm.la \
		@top_builddir@/servers/portmanager/libPortManager.la \
		@SQLITE3_LDFLAGS@ @ZMQ_LDFLAGS@ -Wl,"-rpath=@libdir@"

COMMON_CPPFLAGS=-I@srcdir@/..  @SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqlite++ \
	-I@top_srcdir@/base/uri -I@top_srcdir@/base/os -I@top_srcdir@/servers/portmanager

libvarmgr_la_SOURCES = CVarMgrApi.cpp CVarMgrFileApi.cpp CVarMgrServerApi.cpp \
	CVarMgrApiFactory.cpp

libvarmgr_la_CPPFLAGS=$(COMMON_CPPFLAGS)
libvarmgr_la_LDFLAGS = $(COMMONLDFLAGS)

libvarmgrtcl_la_CPPFLAGS=@TCL_FLAGS@ @LIBTCLPLUS_CFLAGS@ $(COMMON_CPPFLAGS) 
libvarmgrtcl_la_LDFLAGS= @builddir@/libvarmgr.la \
	$(COMMONLDFLAGS) @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@

libvarmgrtcl_la_SOURCES=varmgrTCLPackage.cpp \
	CVarMgrCreateCommand.h CVarMgrCreateCommand.cpp \
	CVarMgrOpenCommand.h   CVarMgrOpenCommand.cpp   \
	CVarMgrCloseCommand.h  CVarMgrCloseCommand.cpp	\
	CVarMgrMkdirCommand.h  CVarMgrMkdirCommand.cpp	\
	CVarMgrCdCommand.h     CVarMgrCdCommand.cpp	\
	CVarMgrGetwdCommand.h  CVarMgrGetwdCommand.cpp	\
	CVarMgrLsCommand.h     CVarMgrLsCommand.cpp	\
	CVarMgrRmdirCommand.h  CVarMgrRmdirCommand.cpp  \
	CVarMgrVarCommand.h    CVarMgrVarCommand.cpp	\
	CVarMgrEnumCommand.h   CVarMgrEnumCommand.cpp	\
	CVarMgrStateMachine.h  CVarMgrStateMachine.cpp


libvarmgrpy_la_CPPFLAGS=$(COMMON_CPPFLAGS) @PYTHON_CPPFLAGS@
libvarmgrpy_la_LDFLAGS= @PYTHON_LDFLAGS@ libvarmgr.la $(libvarmgr_la_LDFLAGS)
libvarmgrpy_la_SOURCES=pyVarMgr.cpp

include_HEADERS=CVarMgrApi.h CVarMgrFileApi.h CVarMgrServerApi.h CVarMgrApiFactory.h


install-exec-hook:
	echo "#!@PYTHON@" >  @bindir@/vardbServer
	cat @srcdir@/vardbServer.py >>@bindir@/vardbServer
	chmod a+x @bindir@/vardbServer
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/varmgr
	$(INSTALL_PROGRAM) @libdir@/libvarmgrtcl.* @prefix@/TclLibs/varmgr
	echo pkg_mkIndex -verbose @prefix@/TclLibs/varmgr "*.so" | @TCLSH_CMD@
	$(mkinstalldirs) @prefix/pythonLibs 
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	$(INSTALL_PROGRAM) @libdir@/libvarmgrpy.so @prefix@/pythonLibs/nscldaq/vardb/varmgr.so


SERVER_TESTS=startTests.py serviceTests.py mkdirTests.py  \
	testGet.py testDirlist.py testVarlist.py testRmvar.py \
	pyVarMgrTests.py

noinst_PROGRAMS = testApi

testApi_SOURCES=TestRunner.cpp TestApiBase.cpp testDirect.cpp \
	testServer.cpp factoryTests.cpp \
	$(libvarmgr_la_SOURCES)
testApi_CPPFLAGS=-DBINDIR=\"@bindir@\" \
	$(COMMON_CPPFLAGS)   \
	 @LIBTCLPLUS_CFLAGS@ \
	@CPPUNIT_CFLAGS@ 
testApi_LDFLAGS = $(COMMONLDFLAGS) \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@	\
	 @CPPUNIT_LDFLAGS@

check-TESTS:
	for f in $(SERVER_TESTS); do echo $$f : ; PYTHONPATH=@prefix@/pythonLibs  BINDIR=@bindir@  @PYTHON@ @srcdir@/$$f; done
	PYTHONPATH=@prefix@/pythonLibs testApi
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ tclbindings.test


EXTRA_DIST=$(SERVER_TESTS) testBase.py vardbServer.py varserver.xml


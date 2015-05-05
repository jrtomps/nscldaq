lib_LTLIBRARIES=libvarmgr.la libvarmgrtcl.la libvarmgrpy.la libnotifierpy.la

COMMONLDFLAGS = @builddir@/../libvardb.la 	\
		@top_builddir@/base/sqlite++/libsqlite3pp.la 	\
		@top_builddir@/base/uri/liburl.la		\
		@top_builddir@/base/os/libdaqshm.la \
		@top_builddir@/servers/portmanager/libPortManager.la \
		@SQLITE3_LDFLAGS@ @ZMQ_LDFLAGS@ -Wl,"-rpath=@libdir@"

COMMON_CPPFLAGS=-I@srcdir@/..  @SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqlite++ \
	-I@top_srcdir@/base/uri -I@top_srcdir@/base/os -I@top_srcdir@/servers/portmanager

libvarmgr_la_SOURCES = CVarMgrApi.cpp CVarMgrFileApi.cpp CVarMgrServerApi.cpp \
	CVarMgrApiFactory.cpp CVarMgrSubscriptions.cpp

include_HEADERS= CVarMgrApi.h CVarMgrFileApi.h CVarMgrServerApi.h CVarMgrApiFactory.h CVarMgrSubscriptions.h



libvarmgr_la_CPPFLAGS=$(COMMON_CPPFLAGS) @LIBTCLPLUS_CFLAGS@
libvarmgr_la_LDFLAGS = $(COMMONLDFLAGS)  @LIBEXCEPTION_LDFLAGS@

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
	CVarMgrStateMachine.h  CVarMgrStateMachine.cpp  \
	CVarMgrSubscribe.h     CVarMgrSubscribe.cpp     \
	CVarMgrSubCommand.h    CVarMgrSubCommand.cpp  


libvarmgrpy_la_CPPFLAGS=$(COMMON_CPPFLAGS) @PYTHON_CPPFLAGS@
libvarmgrpy_la_LDFLAGS= @PYTHON_LDFLAGS@ libvarmgr.la $(libvarmgr_la_LDFLAGS)
libvarmgrpy_la_SOURCES=pyVarMgr.cpp

libnotifierpy_la_CPPFLAGS=$(COMMON_CPPFLAGS) @LIBTCLPLUS_CFLAGS@ @PYTHON_CPPFLAGS@
libnotifierpy_la_LDFLAGS= @top_builddir@/base/uri/liburl.la \
			@top_builddir@/base/os/libdaqshm.la \
			@top_builddir@/servers/portmanager/libPortManager.la \
			@LIBEXCEPTION_LDFLAGS@ \
			@PYTHON_LDFLAGS@ libvarmgr.la $(libvarmgr_la_LDFLAGS)
libnotifierpy_la_SOURCES=pyNotifier.cpp



install-exec-hook:
	echo "#!@PYTHON@" >  @bindir@/vardbServer
	cat @srcdir@/vardbServer.py >>@bindir@/vardbServer
	chmod a+x @bindir@/vardbServer
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/varmgr
	$(INSTALL_PROGRAM) @libdir@/libvarmgrtcl.* @prefix@/TclLibs/varmgr
	echo pkg_mkIndex -verbose @prefix@/TclLibs/varmgr "*.so" | @TCLSH_CMD@
	$(mkinstalldirs) @prefix@/pythonLibs 
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	$(INSTALL_PROGRAM) @libdir@/libvarmgrpy.so @prefix@/pythonLibs/nscldaq/vardb/varmgr.so
	$(INSTALL_PROGRAM) @libdir@/libnotifierpy.so @prefix@/pythonLibs/nscldaq/vardb/notifier.so


SERVER_TESTS=startTests.py serviceTests.py mkdirTests.py  \
	testGet.py testDirlist.py testVarlist.py testRmvar.py \
	pyVarMgrTests.py pyNotifierTests.py

noinst_PROGRAMS = testApi testSub

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



testSub_SOURCES=TestRunner.cpp SubscriptionTests.cpp \
	$(libvarmgr_la_SOURCES)

testSub_CPPFLAGS=-DBINDIR=\"@bindir@\" \
	$(COMMON_CPPFLAGS)   \
	 @LIBTCLPLUS_CFLAGS@ \
	@CPPUNIT_CFLAGS@ 

testSub_LDFLAGS = $(COMMONLDFLAGS) \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@	\
	 @CPPUNIT_LDFLAGS@

check-TESTS:
	for f in $(SERVER_TESTS); do echo $$f : ; PYTHONPATH=@prefix@/pythonLibs  BINDIR=@bindir@  @PYTHON@ @srcdir@/$$f; done
	PYTHONPATH=@prefix@/pythonLibs testApi
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ tclbindings.test
	PYTHONPATH=@prefix@/pythonLibs BINDIR=@prefix@/bin TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ tclsub.test
	PYTHONPATH=@prefix@/pythonLibs testSub

EXTRA_DIST=$(SERVER_TESTS) testBase.py vardbServer.py varserver.xml tclbindings.test tclsub.test


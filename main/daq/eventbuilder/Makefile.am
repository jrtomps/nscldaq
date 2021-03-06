bin_PROGRAMS= frag2ring

COMPILATION_FLAGS =  \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/tcpip \
	@LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/base/tcpip        \
	-I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/os \
	-I@top_srcdir@/base/thread \
	@LIBTCLPLUS_CFLAGS@ \
	@THREADCXX_FLAGS@ @TCL_FLAGS@


lib_LTLIBRARIES=libEventBuilder.la libEventBuilderClient.la 




libEventBuilder_la_SOURCES = OrdererPackage.cpp CFragmentHandlerCommand.cpp CFragmentHandler.cpp \
	CInputStatsCommand.cpp COrdererOutput.cpp fragment.c \
	COutputStatsObserver.cpp COutputStatsCommand.cpp CDataLateStatistics.cpp \
	CDLateStatsCommand.cpp CLateScriptObserver.cpp COnLateDataCommand.cpp  \
	CBarrierStatsCommand.cpp CBarrierStats.cpp CIncompleteBarrierStats.cpp \
	CCompleteBarrierCallback.cpp CBarrierTraceCommand.cpp CPartialBarrierCallback.cpp \
	CSourceCommand.cpp CDeadSourceCommand.cpp CReviveSocketCommand.cpp \
	CFlushCommand.cpp CResetCommand.cpp CConfigure.cpp CDuplicateTimeStatCommand.cpp \
	COutOfOrderTraceCommand.cpp CXonXOffCallbackCommand.cpp COutputThread.cpp

libEventBuilder_la_CPPFLAGS=$(COMPILATION_FLAGS)

libEventBuilderClient_la_SOURCES=CEventOrderClient.cpp fragment.c \
	CEVBClientApp.cpp  CEVBFrameworkApp.cpp \
	EVBFramework.cpp GetOpt.cpp fragio.cpp

libEventBuilderClient_la_CPPFLAGS=$(COMPILATION_FLAGS)

frag2ring_SOURCES=frag2ring.cpp CFragReader.cpp CFragWriter.cpp

frag2ring_CPPFLAGS=$(COMPILATION_FLAGS)

frag2ring_LDADD = @top_builddir@/daq/format/libdataformat.la \
                    @top_builddir@/base/os/libdaqshm.la $(THREADLD_FLAGS)

FRAMEWORK_SOURCEFILES =  GetOpt.cpp Main.cpp options.ggo

FRAMEWORK_SKELETON =  $(FRAMEWORK_SOURCEFILES)


include_HEADERS = CEventOrderClient.h fragment.h CEVBClientApp.h EVBFramework.h \
	CEVBFrameworkApp.h  GetOpt.h fragio.h


noinst_HEADERS = CFragmentHandlerCommand.h CFragmentHandler.h  \
	CInputStatsCommand.h COrdererOutput.h COutputStatsObserver.h COutputStatsCommand.h \
	CDataLateStatistics.h CDLateStatsCommand.h CLateScriptObserver.h COnLateDataCommand.h \
	CBarrierStatsCommand.h CBarrierStats.h CIncompleteBarrierStats.h CCompleteBarrierCallback.h \
	CBarrierTraceCommand.h CPartialBarrierCallback.h CSourceCommand.h CDeadSourceCommand.h \
	CReviveSocketCommand.h CFragReader.h CFragWriter.h CFlushCommand.h CResetCommand.h \
	CConfigure.h fragio.h CDuplicateTimeStatCommand.h CXonXOffCallbackCommand.h \
	COutOfOrderTraceCommand.h COutputThread.h





libEventBuilderClient_la_LIBADD = @top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/servers/portmanager/libPortManager.la   \
	@top_builddir@/base/tcpip/libTcp.la	\
	@top_builddir@/base/os/libdaqshm.la		\
	@LIBTCLPLUS_LDFLAGS@ 

libEventBuilder_la_LIBADD = @LIBTCLPLUS_LDFLAGS@	\
	@top_builddir@/base/thread/libdaqthreads.la 	\
	@top_builddir@/base/os/libdaqshm.la    \
	@TCL_LDFLAGS@ @THREADLD_FLAGS@

libEventBuilder_la_LDFLAGS=@LIBTCLPLUS_LDFLAGS@

TCL_PACKAGE_FILES=ConnectionManager.tcl callbackManager.tcl eventOrderer.tcl \
	connectionList.tcl observer.tcl barriers.tcl GUI.tcl inputStatistics.tcl \
	late.tcl outputStatistics.tcl utility.tcl latePopup.tcl \
	DuplicateTimestamp.tcl OutOfOrderUI.tcl evbui.tcl


TCL_EVB_PACKAGE_FILES=evbRdoCallouts.tcl evbRdoCallouts10.tcl EVBStateCallouts.tcl \
			 ringSourceMgr.tcl endrunmon.tcl

TCL_TEST_FILES=tclTests.tcl cbmanagerTests.test cmanagerCallbacks.test ringsourcemgr.test


EXTRA_DIST=eventbuilder_user.xml protocol.xml api.xml orderer.xml frag2ring.xml Asserts.h \
	$(TCL_PACKAGE_FILES) \
	$(TCL_TEST_FILES) options.ggo \
        $(TCL_EVB_PACKAGE_FILES)       \
	$(FRAMEWORK_SOURCEFILES) \
	Makefile_Skeleton.in startOrderer.in


SKELDIR=@prefix@/skeletons/eventbuilder

install-exec-hook:
	$(mkinstalldirs) $(prefix)/TclLibs/EventBuilder
	for f in $(TCL_PACKAGE_FILES); do @INSTALL_SCRIPT@ @srcdir@/$$f $(prefix)/TclLibs/EventBuilder; done
	rm -f $(prefix)/TclLibs/EventBuilder/libEventBuilder.so
	$(LN_S) @libdir@/libEventBuilder.so $(prefix)/TclLibs/EventBuilder/libEventBuilder.so
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/EventBuilder *.tcl *.so" |@TCLSH_CMD@ 
	$(mkinstalldirs) $(SKELDIR)
	for f in $(FRAMEWORK_SKELETON); do $(INSTALL_DATA) @srcdir@/$$f $(SKELDIR); done
	$(INSTALL_SCRIPT) @builddir@/Makefile_Skeleton $(SKELDIR)/Makefile
	$(INSTALL_SCRIPT) @builddir@/startOrderer @prefix@/bin
	$(mkinstalldirs) @prefix@/TclLibs/EVBCallouts
	for f in $(TCL_EVB_PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/EVBCallouts; done
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/EVBCallouts *.tcl *.so" |@TCLSH_CMD@ 



#-------------------------------------------
#
# Tests:

noinst_PROGRAMS = unittests ordertests

unittests_DEPENDENCIES = libEventBuilderClient.la cmdline.o cmdline.h
unittests_SOURCES = TestRunner.cpp lookupTest.cpp  \
	connectTest.cpp  
nodist_unittests_SOURCES = cmdline.c cmdline.h

unittests_LDADD = @builddir@/libEventBuilderClient.la \
	$(libEventBuilder_la_LIBADD) \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/tcpip/libTcp.la 	\
	@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

unittests_CPPFLAGS=$(COMPILATION_FLAGS)



ordertests_SOURCES = TestRunner.cpp orderTests.cpp duptscmdtest.cpp \
	configcmdtests.cpp tclflowtest.cpp \
	CFragmentHandler.cpp fragment.c CDuplicateTimeStatCommand.cpp \
	CConfigure.cpp CXonXOffCallbackCommand.cpp COutputThread.cpp


ordertests_LDADD = 	@top_builddir@/base/thread/libdaqthreads.la 	\
	@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@ @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@

ordertests_CXXFLAGS=$(COMPILATION_FLAGS) @TCL_CPPFLAGS@ @LIBTCLPLUS_CFLAGS@

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl
	./ordertests
#	./unittests 


BUILT_SOURCES=cmdline.h cmdline.c

cmdline.c: cmdline.h

cmdline.h: options.ggo
	$(GENGETOPT) --input=@srcdir@/options.ggo \
		--output-dir=@builddir@




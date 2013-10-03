bin_PROGRAMS= frag2ring


lib_LTLIBRARIES=libEventBuilder.la  libEventBuilderClient.la 




libEventBuilder_la_SOURCES = OrdererPackage.cpp CFragmentHandlerCommand.cpp CFragmentHandler.cpp \
	CInputStatsCommand.cpp COrdererOutput.cpp fragment.c \
	COutputStatsObserver.cpp COutputStatsCommand.cpp CDataLateStatistics.cpp \
	CDLateStatsCommand.cpp CLateScriptObserver.cpp COnLateDataCommand.cpp  \
	CBarrierStatsCommand.cpp CBarrierStats.cpp CIncompleteBarrierStats.cpp \
	CCompleteBarrierCallback.cpp CBarrierTraceCommand.cpp CPartialBarrierCallback.cpp \
	CSourceCommand.cpp CDeadSourceCommand.cpp CReviveSocketCommand.cpp \
	CFlushCommand.cpp CResetCommand.cpp CConfigure.cpp


libEventBuilderClient_la_SOURCES=CEventOrderClient.cpp fragment.c \
	CEVBClientApp.cpp  CEVBFrameworkApp.cpp \
	EVBFramework.cpp GetOpt.cpp fragio.cpp

frag2ring_SOURCES=frag2ring.cpp CFragReader.cpp CFragWriter.cpp

frag2ring_LDADD = @top_builddir@/base/os/libdaqshm.la $(THREADLD_FLAGS)

FRAMEWORK_SOURCEFILES =  GetOpt.cpp Main.cpp options.ggo

FRAMEWORK_SKELETON = Makefile_Skeleton $(FRAMEWORK_SOURCEFILES)


include_HEADERS = CEventOrderClient.h fragment.h CEVBClientApp.h EVBFramework.h \
	CEVBFrameworkApp.h  GetOpt.h fragio.h



noinst_HEADERS = CFragmentHandlerCommand.h CFragmentHandler.h cmdline.h \
	CInputStatsCommand.h COrdererOutput.h COutputStatsObserver.h COutputStatsCommand.h \
	CDataLateStatistics.h CDLateStatsCommand.h CLateScriptObserver.h COnLateDataCommand.h \
	CBarrierStatsCommand.h CBarrierStats.h CIncompleteBarrierStats.h CCompleteBarrierCallback.h \
	CBarrierTraceCommand.h CPartialBarrierCallback.h CSourceCommand.h CDeadSourceCommand.h \
	CReviveSocketCommand.h CFragReader.h CFragWriter.h CFlushCommand.h CResetCommand.h \
	CConfigure.h fragio.h


INCLUDES = -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/tcpip \
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/base/tcpip        \
	-I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/os \
	@LIBTCLPLUS_CFLAGS@	\
	@THREADCXX_FLAGS@ @TCL_FLAGS@


libEventBuilderClient_la_LIBADD = @top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/servers/portmanager/libPortManager.la   \
	@top_builddir@/base/tcpip/libTcp.la	\
	@top_builddir@/base/os/libdaqshm.la		\
	@LIBEXCEPTION_LDFLAGS@

libEventBuilder_la_LIBADD = @LIBTCLPLUS_LDFLAGS@ \
	@top_builddir@/base/os/libdaqshm.la    \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/base/os/libdaqshm.la \
	@TCL_LDFLAGS@ @THREADLD_FLAGS@

TCL_PACKAGE_FILES=ConnectionManager.tcl callbackManager.tcl eventOrderer.tcl \
	connectionList.tcl observer.tcl barriers.tcl GUI.tcl inputStatistics.tcl \
	late.tcl outputStatistics.tcl utility.tcl

TCL_EVB_PACKAGE_FILES=evbRdoCallouts.tcl

TCL_TEST_FILES=tclTests.tcl cbmanagerTests.test cmanagerCallbacks.test


EXTRA_DIST=protocol.xml api.xml orderer.xml frag2ring.xml Asserts.h \
	$(TCL_PACKAGE_FILES) \
	$(TCL_TEST_FILES) options.ggo \
        $(TCL_EVB_PACKAGE_FILES)       \
	$(FRAMEWORK_SOURCEFILES) \
	Makefile_Skeleton.in startOrderer.in


SKELDIR=@prefix@/skeletons/eventbuilder

install-exec-hook:
	$(mkinstalldirs) $(prefix)/TclLibs/EventBuilder
	$(INSTALL_SCRIPT) $(TCL_PACKAGE_FILES) $(prefix)/TclLibs/EventBuilder
	rm -f $(prefix)/TclLibs/EventBuilder/libEventBuilder.so
	$(LN_S) @libdir@/libEventBuilder.so $(prefix)/TclLibs/EventBuilder/libEventBuilder.so
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/EventBuilder *.tcl *.so" |@TCLSH_CMD@ 
	$(mkinstalldirs) $(SKELDIR)
	$(INSTALL_DATA) $(FRAMEWORK_SKELETON) $(SKELDIR)
	mv $(SKELDIR)/Makefile_Skeleton $(SKELDIR)/Makefile
	$(INSTALL_SCRIPT) startOrderer @prefix@/bin
	$(mkinstalldirs) @prefix@/TclLibs/EVBCallouts
	$(INSTALL_SCRIPT) $(TCL_EVB_PACKAGE_FILES) @prefix@/TclLibs/EVBCallouts
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/EVBCallouts *.tcl *.so" |@TCLSH_CMD@ 



#-------------------------------------------
#
# Tests:

noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp lookupTest.cpp  \
	connectTest.cpp  cmdline.c


unittests_LDADD = libEventBuilderClient.la \
	$(libEventBuilder_la_LIBADD) \
	@top_builddir@/servers/portmanager/libPortManager.la \
	@top_builddir@/base/tcpip/libTcp.la 	\
	@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@


unittests_DEPENDENCIES = cmdline.o cmdline.h

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ tclTests.tcl
#	./unittests 


BUILT_SOURCES=cmdline.h cmdline.c

cmdline.h: options.ggo
	$(GENGETOPT) --input=options.ggo

cmdline.c: options.ggo
	$(GENGETOPT) --input=options.ggo





INCLUDES = -I$(top_srcdir)/base/headers @TCL_FLAGS@ -I$(top_srcdir)/base/exception
LDADD = -ltclPlus -lException -lXt -lX11 $(X11EXTRA_LDFLAGS)
AM_LDFLAGS = -L. @TCL_LDFLAGS@ -L$(top_srcdir)/base/exception

lib_LTLIBRARIES = libtclPlus.la

libtclPlus_la_LDFLAGS = -version-info $(SOVERSION) \
	-Wl,"-rpath-link=$(libdir)" $(THREADLD_FLAGS)

libtclPlus_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

testableSources = TCLInterpreter.cpp TCLProcessor.cpp  \
	TCLVariable.cpp TCLString.cpp  \
	TCLResult.cpp TCLTimer.cpp TCLFileHandler.cpp \
	TCLList.cpp TCLException.cpp TCLInterpreterObject.cpp \
	TCLCommandPackage.cpp TCLIdleProcess.cpp \
	TCLObject.cpp TCLChannel.cpp TCLTracedVariable.cpp \
	CLanguageTraceCallbacks.cpp TCLObjectProcessor.cpp \
	TCLCompatibilityProcessor.cpp TCLLiveEventLoop.cpp	\
	CTCLChannelCommander.cpp CTCLStdioCommander.cpp	\
	CTCLTcpServerInstance.cpp CTCLServer.cpp TCLObjectPackage.cpp \
	TCLPackagedObjectProcessor.cpp  CItemConfiguration.cpp \
	CConfigurableObject.cpp

testableObjects = TCLInterpreter.lo   TCLProcessor.lo    \
	TCLVariable.lo   TCLString.lo    \
	TCLResult.lo   TCLTimer.lo   TCLFileHandler.lo   \
	TCLList.lo   TCLException.lo \
	TCLInterpreterObject.lo   \
	TCLCommandPackage.lo   TCLIdleProcess.lo   \
	TCLObject.lo   TCLChannel.lo   TCLTracedVariable.lo   \
	CLanguageTraceCallbacks.lo  TCLObjectProcessor.lo \
	TCLCompatibilityProcessor.lo TCLPackagedObjectProcessor.lo \
	TCLObjectPackage.lo


libtclPlus_la_SOURCES = TCLApplication.cpp $(testableSources)



include_HEADERS = TCLApplication.h TCLCommandPackage.h TCLException.h \
	TCLFileHandler.h TCLHashTable.h TCLHashTableItem.h \
	TCLHashTableIterator.h TCLIdleProcess.h	TCLInterpreter.h \
	TCLInterpreterObject.h TCLList.h TCLObject.h TCLPackagedCommand.h \
	TCLProcessor.h TCLResult.h TCLString.h TCLTimer.h TCLVariable.h \
	TCLChannel.h TCLVersionHacks.h TCLTracedVariable.h \
	VariableTraceCallback.h CLanguageTraceCallbacks.h \
	TCLObjectProcessor.h TCLCompatibilityProcessor.h 	\
	TCLLiveEventLoop.h CTCLChannelCommander.h	\
	CTCLStdioCommander.h CTCLTcpServerInstance.h CTCLServer.h	\
	TCLObjectPackage.h TCLPackagedObjectProcessor.h \
	CItemConfiguration.h CConfigurableObject.h


# evttclsh - a tclsh that has an event loop constantly running.


bin_PROGRAMS		=	evttclsh

evttclsh_SOURCES	= 	evttclsh.cpp
evttclsh_DEPENDENCIES	= 	libtclPlus.la

evttclsh_LDADD		= 	libtclPlus.la $(TCL_LDFLAGS)	\
				@top_srcdir@/base/exception/libException.la
evttclsh_LDFLAGS	= 	-Wl,"-rpath-link=$(libdir)" $(TCL_LDFLAGS)


clean-local:
	-rm -f $(XMLMANS)

EXTRA_DIST = tkAppInit.c tclplus.xml


noinst_PROGRAMS = tracetests interptests tcpservertest

noinst_HEADERS = Asserts.h



tracetests_SOURCES = TestRunner.cpp \
			ccallbacktest.cpp \
			tracevartest.cpp

tracetests_DEPENDENCIES = $(testableObjects)

TestLdflags = $(testableObjects) $(CPPUNIT_LDFLAGS) $(X11EXTRA_LDFLAGS) \
	-L@top_srcdir@/base/exception -lException


tracetests_LDADD = $(TestLdflags) $(TCL_LDFLAGS)

interptests_SOURCES=interpretertest.cpp objecttest.cpp resulttest.cpp \
	objcommandtest.cpp compatcommand.cpp packagetests.cpp


interptests_LDADD = TestRunner.lo $(TestLdflags)


interptests_DEPENDENCIES = $(testableObjects) TestRunner.lo

tcpservertest_SOURCES	= 	tcpservertest.cpp
tcpservertest_DEPENDENCIES	= 	libtclPlus.la

tcpservertest_LDADD		= 	libtclPlus.la	\
				@top_srcdir@/base/exception/libException.la \
			$(TCL_LDFLAGS)
tcpservertest_LDFLAGS	= 	-Wl,"-rpath-link=$(libdir)" $(TCL_LDFLAGS)



TESTS = ./tracetests ./interptests





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
	CTCLTcpServerInstance.cpp

testableObjects = TCLInterpreter.o   TCLProcessor.o    \
	TCLVariable.o   TCLString.o    \
	TCLResult.o   TCLTimer.o   TCLFileHandler.o   \
	TCLList.o   TCLException.o \
	TCLInterpreterObject.o   \
	TCLCommandPackage.o   TCLIdleProcess.o   \
	TCLObject.o   TCLChannel.o   TCLTracedVariable.o   \
	CLanguageTraceCallbacks.o  TCLObjectProcessor.o \
	TCLCompatibilityProcessor.o

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
	CTCLStdioCommander.h CTCLTcpServerInstance.h


# evttclsh - a tclsh that has an event loop constantly running.


bin_PROGRAMS		=	evttclsh

evttclsh_SOURCES	= 	evttclsh.cpp
evttclsh_DEPENDENCIES	= 	libtclPlus.la

evttclsh_LDADD		= 	libtclPlus.la	\
				@top_srcdir@/base/exception/libException.la
evttclsh_LDFLAGS	= 	-Wl,"-rpath-link=$(libdir)" $(TCL_LDFLAGS)

clean-local:
	-rm -f $(XMLMANS)

EXTRA_DIST = tkAppInit.c tclplus.xml


noinst_PROGRAMS = tracetests interptests

noinst_HEADERS = Asserts.h



tracetests_SOURCES = TestRunner.cpp \
			ccallbacktest.cpp \
			tracevartest.cpp

tracetests_DEPENDENCIES = $(testableObjects)

TestLdflags = $(testableObjects) $(CPPUNIT_LDFLAGS) $(X11EXTRA_LDFLAGS) \
	-L@top_srcdir@/base/exception -lException


tracetests_LDADD = $(TestLdflags)

interptests_SOURCES=interpretertest.cpp objecttest.cpp resulttest.cpp \
	objcommandtest.cpp compatcommand.cpp


interptests_LDADD = TestRunner.o $(TestLdflags)


interptests_DEPENDENCIES = $(testableObjects) TestRunner.o


TESTS = ./tracetests ./interptests


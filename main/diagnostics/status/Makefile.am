bin_PROGRAMS = ringstatusdaemon statusaggregator multiaggregator

lib_LTLIBRARIES = libStatusMessages.la libTclStatusMessages.la libPyStatusMessages.la

COMPILATION_FLAGS= -I@top_srcdir@/base/os -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/servers/portmanager \
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@
COMMON_LIBS      = @top_builddir@/base/os/libdaqshm.la  \
	@top_builddir@/base/dataflow/libDataFlow.la	\
	@top_builddir@/servers/portmanager/libPortManager.la \
	@ZMQ_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@

libStatusMessages_la_SOURCES=Constants.cpp CZmqContext.cpp \
	CRingStatisticsMessage.cpp \
	CReadoutStatistics.cpp \
	CStatusDefinitions.cpp \
	LogMessage.cpp		\
	StateChange.cpp		\
	CStatusSubscription.cpp  \
	CPublishRingStatistics.cpp CMultiAggregator.cpp 



libStatusMessages_la_CPPFLAGS = $(COMPILATION_FLAGS)
libStatusMessages_la_LDFLAGS  = $(COMMON_LIBS)

include_HEADERS = CStatusMessage.h CPublishRingStatistics.h \
	CStatusSubscription.h CMultiAggregator.h

#------------------- daemon for ring status:

ringstatusdaemon_SOURCES  = ringStatusDaemon.cpp \
	CRingStatusDaemon.cpp CRingStatusDaemon.h

ringstatusdaemon_CPPFLAGS = -I@top_srcdir@/servers/portmanager @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/dataflow
ringstatusdaemon_LDFLAGS  = @top_builddir@/servers/portmanager/libPortManager.la \
	@builddir@/libStatusMessages.la	@top_builddir@/base/dataflow/libDataFlow.la \
	@LIBEXCEPTION_LDFLAGS@

nodist_ringstatusdaemon_SOURCES = ringstatus.c ringstatus.h


#---------------  status aggregator:


statusaggregator_SOURCES  = aggregator.cpp
statusaggregator_CPPFLAGS = -I@top_srcdir@/servers/portmanager @LIBTCLPLUS_CFLAGS@
statusaggregator_LDFLAGS  =  @top_builddir@/servers/portmanager/libPortManager.la \
	@builddir@/libStatusMessages.la						  \
	@ZMQ_LDFLAGS@								\
	@LIBEXCEPTION_LDFLAGS@

#--------------- aggregator with node discovery.

multiaggregator_SOURCES = multiaggmain.cpp CMultiAggregator.cpp CMultiAggregator.h
multiaggregator_CPPFLAGS=-I@top_srcdir@/servers/portmanager @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/dataflow -I@top_srcdir@/base/os
multiaggregator_LDFLAGS = @top_builddir@/servers/portmanager/libPortManager.la \
	@builddir@/libStatusMessages.la @top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la					\
	@ZMQ_LDFLAGS@								\
	@LIBEXCEPTION_LDFLAGS@

nodist_multiaggregator_SOURCES = multiaggregator.h multiaggregator.c


#---------------- build ggo files into .c/.h files.

BUILT_SOURCES=ringstatus.c ringstatus.h multiaggregator.c multiaggregator.h

ringstatus.c: ringstatus.h

ringstatus.h: ringstatus.ggo
	$(GENGETOPT) --output-dir=@builddir@ --input=@srcdir@/ringstatus.ggo -F ringstatus

multiaggregator.c: multiaggregator.h

multiaggregator.h: multiaggregator.ggo
	$(GENGETOPT) --output-dir=@builddir@ --input=@srcdir@/multiaggregator.ggo -F multiaggregator


#--------------------Tcl Bindings:

libTclStatusMessages_la_SOURCES= TclPackage.cpp             \
	CTCLRingStatistics.cpp CTCLRingStatistics.h         \
	CTCLReadoutStatistics.cpp CTCLReadoutStatistics.h   \
	CTCLLogMessage.cpp CTCLLogMessage.h		    \
	CTCLStateChangeMessage.cpp CTCLStateChangeMessage.h \
	CTCLSubscriptions.cpp CTCLSubscriptions.h             \
	CTCLDecodeMessage.cpp CTCLDecodeMessage.h	\
	CTCLAggregate.cpp CTCLAggregate.h		\
	TclUtilities.cpp TclUtilities.h 

libTclStatusMessages_la_CPPFLAGS=$(COMPILATION_FLAGS) @TCL_FLAGS@ @LIBTCLPLUS_CFLAGS@ @TCL_DEFS@
libTclStatusMessages_la_LDFLAGS=$(COMMON_LIBS)  	\
				@builddir@/libStatusMessages.la   \
				@LIBTCLPLUS_LDFLAGS@ 	\
				@TCL_LDFLAGS@

tclpkgInstall:
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/status
	rm -f @prefix@/TclLibs/status/libTclStatusMessages.so
	$(LN_S) @libdir@/libTclStatusMessages.so @prefix@/TclLibs/status/libTclStatusMessages.so
	echo pkg_mkIndex -verbose "@prefix@/TclLibs/status *.so" |@TCLSH@

tclAppInstall::
	$(mkinstalldirs) @bindir@
	$(INSTALL_SCRIPT) Log.tcl @bindir@/Log


#------------------- Python Bindings -

libPyStatusMessages_la_SOURCES= PyStatusMessages.cpp
libPyStatusMessages_la_CPPFLAGS=$(COMPILATION_FLAGS) @PYTHON_CPPFLAGS@
libPyStatusMessages_la_LDFLAGS=$(COMMON_LIBS) @builddir@/libStatusMessages.la \
				@PYTHON_LDFLAGS@

pypkgInstall:
	$(mkinstalldirs) @prefix@/pythonLibs  @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/status
	touch @prefix@/pythonLibs/nscldaq/status/__init__.py
	$(INSTALL_PROGRAM) @libdir@/libPyStatusMessages.so @prefix@/pythonLibs/nscldaq/status/statusmessages.so

#-------------------- Tests -----

TEST_EXTENSIONS=.py .test
PY_LOG_COMPILER=@PYTHON@
TEST_LOG_COMPILER=@TCLSH@
TESTS_ENVIRONMENT=PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH;\
	TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH;


PYTHON_TESTS=ringstatisticstests.py readoutstatisticstests.py \
	logmessagestests.py statemessagetests.py statussub.py


TCL_TESTS=statussubtest.test ringdecode.test


TEST_PROGS=ringstattests tclpkgtests subtests ringpubclasstest ringdaemontests

noinst_PROGRAMS=$(TEST_PROGS) testringstatus


ringstattests_SOURCES=TestRunner.cpp Asserts.h ringstattests.cpp \
	readoutstattests.cpp logmsgtests.cpp statemsgtests.cpp testutils.cpp testutils.h

ringstattests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
ringstattests_LDFLAGS=libStatusMessages.la $(COMMON_LIBS) @CPPUNIT_LDFLAGS@

tclpkgtests_SOURCES = TestRunner.cpp Asserts.h pkgloads.cpp \
	tclringstatustests.cpp testutils.cpp testutils.h \
	tclreadoutstatustests.cpp tcllogmsgtests.cpp tclstatemsgtests.cpp

tclpkgtests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@ @LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ \
					-DTCLLIBPATH="\"@prefix@/TclLibs\""
tclpkgtests_LDFLAGS=$(COMMON_LIBS) libTclStatusMessages.la \
		libStatusMessages.la	\
		@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@ @CPPUNIT_LDFLAGS@

subtests_SOURCES=TestRunner.cpp subtest.cpp
subtests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
subtests_LDFLAGS=libStatusMessages.la $(COMMON_LIBS) @CPPUNIT_LDFLAGS@

ringpubclasstest_SOURCES=TestRunner.cpp ringpubclasstest.cpp testutils.cpp testutils.h
ringpubclasstest_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
ringpubclasstest_LDFLAGS  =libStatusMessages.la $(COMMON_LIBS) @CPPUNIT_LDFLAGS@

ringdaemontests_SOURCES=TestRunner.cpp ringdaemontests.cpp \
	CRingStatusDaemon.cpp CRingStatusDaemon.h testutils.cpp
ringdaemontests_CPPFLAGS = $(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
ringdaemontests_LDFLAGS  = libStatusMessages.la $(COMMON_LIBS) \
	@CPPUNIT_LDFLAGS@


#  These are not automated tests:

testringstatus_SOURCES = testringstatus.cpp CMultiAggregator.cpp CMultiAggregator.h
testringstatus_CPPFLAGS= $(COMPILATION_FLAGS) -I@top_srcdir@/servers/portmanager
testringstatus_LDFLAGS= libStatusMessages.la \
	@top_builddir@/servers/portmanager/libPortManager.la \
	$(COMMON_LIBS)

TESTS=$(TEST_PROGS) $(PYTHON_TESTS) $(TCL_TESTS)


install-exec-hook: tclpkgInstall pypkgInstall tclAppInstall

clean-local:
	rm -f $(BUILT_SOURCES)

#  Ensure stuff makes it into the tarball:

EXTRA_DIST = $(PYTHON_TESTS) $(TCL_TESTS) ringstatus.ggo multiaggregator.ggo \
	Log.tcl

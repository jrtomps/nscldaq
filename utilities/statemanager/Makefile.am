#
# Primary targets are the APIS used by clients and managers:

lib_LTLIBRARIES = libstateclient.la

PREPROCESSOR=cpp

#  Common flag definitions so even the Makefile.am is DRY:

COMMON_CPPFLAGS=-I@top_srcdir@/utilities/varmanager/server -I@top_srcdir@/base/thread \
		-I@top_srcdir@/base/os -I@top_srcdir@/base/uri                        \
		-I@top_srcdir@/base/thread -I@top_srcdir@/servers/portmanager         \
		-I@top_srcdir@/utilities/varmanager -I@top_srcdir@/base/sqlite++ 	\
		@SQLITE3_CFLAGS@							\
		@LIBTCLPLUS_CFLAGS@ @GLIB_CFLAGS@

COMMON_LOADFLAGS= @top_builddir@/utilities/varmanager/server/libvarmgr.la \
		@top_builddir@/utilities/varmanager/libvardb.la \
		@top_builddir@/servers/portmanager/libPortManager.la \
		@top_builddir@/base/thread/libdaqthreads.la      \
		@top_builddir@/base/os/libdaqshm.la		\
		@top_builddir@/base/sqlite++/libsqlite3pp.la  @SQLITE3_LDFLAGS@  \
		@LIBEXCEPTION_LDFLAGS@ @GLIB_LIBS@


#

libstateclient_la_SOURCES=CStateClientApi.cpp CStateTransitionMonitor.cpp CStateManager.cpp
libstateclient_la_CPPFLAGS=$(COMMON_CPPFLAGS)
libstateclient_la_LDFALGS=$(COMMON_LOADFLAGS)


# Headers for public APIS:

include_HEADERS = CStateClientApi.h CStateTransitionMonitor.h CStateManager.h

#
#  Programs that depend on these libraries that provide command line control over
#  the system:


bin_PROGRAMS=status begin pause resume
noinst_HEADERS=tracker.h

status_SOURCES=status.cpp  @builddir@/statusopts.h
status_DEPENDENCIES = @builddir@/statusopts.o 
status_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
status_LDADD=@builddir@/statusopts.o 
status_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

statusopts.h: @builddir@/statusopts.o



statusopts.o: @builddir@/statusopts.ggo
	$(GENGETOPT) <@builddir@/statusopts.ggo --file=@builddir@/statusopts
	$(CC) -c -o @builddir@/statusopts.o @builddir@/statusopts.c

#  This mess is done so that statusopts.ggi can #include standardopts.ggo for common 
#  program options.

@builddir@/statusopts.ggo: statusopts.ggi standardopts.ggo
	$(PREPROCESSOR) -P @srcdir@/statusopts.ggi  |tr -d "\\\\" >@builddir@/statusopts.ggo

begin_SOURCES=begin.cpp @builddir@/beginopts.h tracker.cpp
begin_DEPENDENCIES=@builddir@/beginopts.o
begin_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
begin_LDADD=beginopts.o
begin_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

beginopts.h: @builddir@/beginopts.o



beginopts.o: @builddir@/beginopts.ggo
	$(GENGETOPT) <@builddir@/beginopts.ggo --file=@builddir@/beginopts
	$(CC) -c -o @builddir@/beginopts.o @builddir@/beginopts.c


@builddir@/beginopts.ggo: beginopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/beginopts.ggi  |tr -d "\\\\" >@builddir@/beginopts.ggo


pause_SOURCES=pause.cpp @builddir@/pauseopts.h tracker.cpp
pause_DEPENDENCIES=@builddir@/pauseopts.o
pause_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
pause_LDADD=pauseopts.o
pause_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

pauseopts.h: @builddir@/pauseopts.o



pauseopts.o: @builddir@/pauseopts.ggo
	$(GENGETOPT) <@builddir@/pauseopts.ggo --file=@builddir@/pauseopts
	$(CC) -c -o @builddir@/pauseopts.o @builddir@/pauseopts.c


@builddir@/pauseopts.ggo: pauseopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/pauseopts.ggi  |tr -d "\\\\" >@builddir@/pauseopts.ggo



resume_SOURCES=resume.cpp @builddir@/resumeopts.h tracker.cpp
resume_DEPENDENCIES=@builddir@/resumeopts.o
resume_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
resume_LDADD=resumeopts.o
resume_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

resumeopts.h: @builddir@/resumeopts.o



resumeopts.o: @builddir@/resumeopts.ggo
	$(GENGETOPT) <@builddir@/resumeopts.ggo --file=@builddir@/resumeopts
	$(CC) -c -o @builddir@/resumeopts.o @builddir@/resumeopts.c


@builddir@/resumeopts.ggo: resumeopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/resumeopts.ggi  |tr -d "\\\\" >@builddir@/resumeopts.ggo






EXTRA_DIST=statusopts.ggi  beginopts.ggi transitionopts.ggi  standardopts.ggo \
	resumeopts.ggi 


#---------------------------------------------------------------------------------
# tests:
#


noinst_PROGRAMS=unittests stateTests

unittests_SOURCES=TestRunner.cpp Asserts.h TestStateChangeMonitor.cpp \
	changeMonitorTests.cpp TestStateManager.cpp

unittests_CPPFLAGS=$(COMMON_CPPFLAGS) $(CPPUNIT_CFLAGS) -DBINDIR=\"@bindir@\"
unittests_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) $(CPPUNIT_LDFLAGS)

stateTests_SOURCES=TestRunner.cpp Asserts.h TestStateTransitions.cpp

stateTests_CPPFLAGS=$(COMMON_CPPFLAGS) $(CPPUNIT_CFLAGS) -DBINDIR=\"@bindir@\"
stateTests_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) $(CPPUNIT_LDFLAGS)



check-TESTS:
	BINDIR="@bindir@" PYTHONPATH=@prefix@/pythonLibs @builddir@/unittests
	BINDIR="@bindir@" PYTHONPATH=@prefix@/pythonLibs @builddir@/stateTests
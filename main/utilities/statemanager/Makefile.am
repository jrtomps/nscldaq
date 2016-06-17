#
# Primary targets are the APIS used by clients and managers:

lib_LTLIBRARIES = libstateclient.la libTclstateclient.la libPyStateManager.la  libPyStateClient.la

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

libstateclient_la_SOURCES=CStateClientApi.cpp CStateTransitionMonitor.cpp CStateManager.cpp \
			CStateProgram.cpp
libstateclient_la_CPPFLAGS=$(COMMON_CPPFLAGS)
libstateclient_la_LDFLAGS=$(COMMON_LOADFLAGS)

#

libTclstateclient_la_SOURCES=CTCLStateClientCommand.cpp CTCLStateClientInstanceCommand.cpp \
	CTCLStateManagerCommand.cpp CTCLStateManagerInstanceCommand.cpp                    \
	TCLStateClientPackage.cpp							   \
	CTCLStateClientCommand.h CTCLStateClientInstanceCommand.h                          \
	CTCLStateManagerCommand.h CTCLStateManagerInstanceCommand.h

libTclstateclient_la_CPPFLAGS=$(COMMON_CPPFLAGS)   @TCL_FLAGS@
libTclstateclient_la_LDFLAGS= libstateclient.la $(COMMON_LOADFLAGS) @LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@


# 



libPyStateManager_la_SOURCES= PyStateManager.cpp PyStateManager.h
libPyStateManager_la_CPPFLAGS=@PYTHON_CPPFLAGS@ $(COMMON_CPPFLAGS)
libPyStateManager_la_LIBADD = libstateclient.la
libPyStateManager_la_LDFLAGS= @PYTHON_LDFLAGS@ $(COMMON_LOADFLAGS) 

libPyStateClient_la_SOURCES= PyStateClient.cpp 
libPyStateClient_la_CPPFLAGS=@PYTHON_CPPFLAGS@ $(COMMON_CPPFLAGS)
libPyStateClient_la_LIBADD = libPyStateManager.la libstateclient.la
libPyStateClient_la_LDFLAGS= @PYTHON_LDFLAGS@ $(COMMON_LOADFLAGS) 


# Headers for public APIS:

include_HEADERS = CStateClientApi.h CStateTransitionMonitor.h CStateManager.h CStateProgram.h



#
#  Programs that depend on these libraries that provide command line control over
#  the system:


bin_PROGRAMS=status begin prebegin prepause pause resume preend end title trtimeout \
	recording programdir runnum preresume
noinst_HEADERS=tracker.h


EXTRA_DIST=statusopts.ggi beginopts.ggi prebeginopts.ggi preendopts.ggi \
		resumeopts.ggi endopts.ggi titleopts.ggi pauseopts.ggi  \
		transitionopts.ggi timeoutopts.ggi recordingopts.ggi \
		programdiropts.ggi runnumopts.ggi \
		 standardopts.ggo  prepauseopts.ggi preresumeopts.ggi

BUILT_SOURCES=statusopts.h beginopts.h resumeopts.h endopts.h titleopts.h pauseopts.h timeoutopts.h \
	recordingopts.h programdiropts.h runnumopts.h prebeginopts.h preendopts.h prepauseopts.h \
	preresumeopts.h

clean-local:
	rm -f $(BUILT_SOURCES)
	rm -f statusopts.ggo endopts.ggo titleopts.ggo pauseopts.ggo transitionopts.ggo
	rm -f timeoutopts.ggo recordingopts.ggo programdiropts.ggo runnumopts.ggo
	rm -f resumeopts.ggo beginopts.ggo prebeginopts.ggo
	rm -f statusopts.c endopts.c titleopts.c pauseopts.c transitionopts.c
	rm -f timeoutopts.c recordingopts.c programdiropts.c runnumopts.c
	rm -f resumeopts.c beginopts.c
	rm -f prebeginopts.c prebeginopts.o preendopts.{c,h,o,ggo}
	rm -f prepauseopts.{c,h,o,ggo} preresumeopts.{c,h,o,ggo}
	rm -f statusopts.o endopts.o titleopts.o pauseopts.o transitionopts.o
	rm -f timeoutopts.o recordingopts.o programdiropts.o runnumopts.o
	rm -f resumeopts.o beginopts.o 



status_SOURCES=status.cpp  statusopts.h
status_DEPENDENCIES = statusopts.o 
status_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
status_LDADD=@builddir@/statusopts.o 
status_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

statusopts.h: statusopts.o



statusopts.o: @builddir@/statusopts.ggo
	$(GENGETOPT) <@builddir@/statusopts.ggo --file=@builddir@/statusopts
	$(CC) -c -o @builddir@/statusopts.o @builddir@/statusopts.c

#  This mess is done so that statusopts.ggi can #include standardopts.ggo for common 
#  program options.

@builddir@/statusopts.ggo: statusopts.ggi standardopts.ggo
	$(PREPROCESSOR) -P @srcdir@/statusopts.ggi  |tr -d "\\\\" >@builddir@/statusopts.ggo

prebegin_SOURCES=prebegin.cpp prebeginopts.h tracker.cpp transition.cpp transition.h tracker.h
prebegin_DEPENDENCIES=prebeginopts.o
prebegin_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir
prebegin_LDADD=prebeginopts.o
prebegin_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 


prebeginopts.h: @builddir@/prebeginopts.o

prebeginopts.o: @builddir@/prebeginopts.ggo
	$(GENGETOPT) <@builddir@/prebeginopts.ggo --file=@builddir@/prebeginopts
	$(CC) -c -o @builddir@/prebeginopts.o @builddir@/prebeginopts.c


@builddir@/prebeginopts.ggo: prebeginopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/prebeginopts.ggi  |tr -d "\\\\" >@builddir@/prebeginopts.ggo


begin_SOURCES=begin.cpp beginopts.h tracker.cpp transition.cpp transition.h
begin_DEPENDENCIES=beginopts.o
begin_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
begin_LDADD=beginopts.o
begin_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

beginopts.h: @builddir@/beginopts.o


beginopts.o: @builddir@/beginopts.ggo
	$(GENGETOPT) <@builddir@/beginopts.ggo --file=@builddir@/beginopts
	$(CC) -c -o @builddir@/beginopts.o @builddir@/beginopts.c


@builddir@/beginopts.ggo: beginopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/beginopts.ggi  |tr -d "\\\\" >@builddir@/beginopts.ggo


pause_SOURCES=pause.cpp pauseopts.h tracker.cpp transition.cpp tracker.h transition.h
pause_DEPENDENCIES=pauseopts.o
pause_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
pause_LDADD=pauseopts.o
pause_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

pauseopts.h: pauseopts.o



pauseopts.o: @builddir@/pauseopts.ggo
	$(GENGETOPT) <@builddir@/pauseopts.ggo --file=@builddir@/pauseopts
	$(CC) -c -o @builddir@/pauseopts.o @builddir@/pauseopts.c


@builddir@/pauseopts.ggo: pauseopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/pauseopts.ggi  |tr -d "\\\\" >@builddir@/pauseopts.ggo




preresume_SOURCES=preresume.cpp preresumeopts.h tracker.cpp transition.cpp transition.h tracker.h
preresume_DEPENDENCIES=preresumeopts.o
preresume_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir
preresume_LDADD=preresumeopts.o
preresume_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 


preresumeopts.h: @builddir@/preresumeopts.o

preresumeopts.o: @builddir@/preresumeopts.ggo
	$(GENGETOPT) <@builddir@/preresumeopts.ggo --file=@builddir@/preresumeopts
	$(CC) -c -o @builddir@/preresumeopts.o @builddir@/preresumeopts.c


@builddir@/preresumeopts.ggo: preresumeopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/preresumeopts.ggi  |tr -d "\\\\" >@builddir@/preresumeopts.ggo


resume_SOURCES=resume.cpp resumeopts.h tracker.cpp transition.h transition.cpp

resume_DEPENDENCIES=resumeopts.o
resume_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
resume_LDADD=resumeopts.o
resume_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

resumeopts.h: resumeopts.o



resumeopts.o: @builddir@/resumeopts.ggo
	$(GENGETOPT) <@builddir@/resumeopts.ggo --file=@builddir@/resumeopts
	$(CC) -c -o @builddir@/resumeopts.o @builddir@/resumeopts.c


@builddir@/resumeopts.ggo: resumeopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/resumeopts.ggi  |tr -d "\\\\" >@builddir@/resumeopts.ggo

preend_SOURCES=preend.cpp preendopts.h tracker.cpp transition.cpp transition.h tracker.h
preend_DEPENDENCIES=preendopts.o
preend_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir
preend_LDADD=preendopts.o
preend_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 


preendopts.h: @builddir@/preendopts.o

preendopts.o: @builddir@/preendopts.ggo
	$(GENGETOPT) <@builddir@/preendopts.ggo --file=@builddir@/preendopts
	$(CC) -c -o @builddir@/preendopts.o @builddir@/preendopts.c


@builddir@/preendopts.ggo: @srcdir@/preendopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/preendopts.ggi | tr -d "\\\\" > @builddir@/preendopts.ggo



prepause_SOURCES=prepause.cpp prepauseopts.h tracker.cpp transition.cpp transition.h tracker.h
prepause_DEPENDENCIES=prepauseopts.o
prepause_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir
prepause_LDADD=prepauseopts.o
prepause_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 


prepauseopts.h: @builddir@/prepauseopts.o

prepauseopts.o: @builddir@/prepauseopts.ggo
	$(GENGETOPT) <@builddir@/prepauseopts.ggo --file=@builddir@/prepauseopts
	$(CC) -c -o @builddir@/prepauseopts.o @builddir@/prepauseopts.c


@builddir@/prepauseopts.ggo: @srcdir@/prepauseopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/prepauseopts.ggi | tr -d "\\\\" > @builddir@/prepauseopts.ggo



end_SOURCES=end.cpp endopts.h tracker.cpp transition.cpp tracker.h transition.h
end_DEPENDENCIES=endopts.o
end_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
end_LDADD=endopts.o
end_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

endopts.h: endopts.o



endopts.o: @builddir@/endopts.ggo
	$(GENGETOPT) <@builddir@/endopts.ggo --file=@builddir@/endopts
	$(CC) -c -o @builddir@/endopts.o @builddir@/endopts.c


@builddir@/endopts.ggo: endopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/endopts.ggi  |tr -d "\\\\" >@builddir@/endopts.ggo


title_SOURCES=title.cpp titleopts.h tracker.cpp
title_DEPENDENCIES=titleopts.o
title_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
title_LDADD=titleopts.o
title_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

titleopts.h: titleopts.o



titleopts.o: @builddir@/titleopts.ggo
	$(GENGETOPT) <@builddir@/titleopts.ggo --file=@builddir@/titleopts --unamed-opts=STRING
	$(CC) -c -o @builddir@/titleopts.o @builddir@/titleopts.c


@builddir@/titleopts.ggo: titleopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/titleopts.ggi  |tr -d "\\\\" >@builddir@/titleopts.ggo


trtimeout_SOURCES=timeout.cpp timeoutopts.h tracker.cpp
trtimeout_DEPENDENCIES=timeoutopts.o
trtimeout_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
trtimeout_LDADD=timeoutopts.o
trtimeout_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

timeoutopts.h: timeoutopts.o



timeoutopts.o: @builddir@/timeoutopts.ggo
	$(GENGETOPT) <@builddir@/timeoutopts.ggo --file=@builddir@/timeoutopts --unamed-opts=TIMEOUT
	$(CC) -c -o @builddir@/timeoutopts.o @builddir@/timeoutopts.c


@builddir@/timeoutopts.ggo: timeoutopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/timeoutopts.ggi  |tr -d "\\\\" >@builddir@/timeoutopts.ggo


recording_SOURCES=recording.cpp recordingopts.h tracker.cpp
recording_DEPENDENCIES=recordingopts.o
recording_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
recording_LDADD=recordingopts.o
recording_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

recordingopts.h: recordingopts.o



recordingopts.o: @builddir@/recordingopts.ggo
	$(GENGETOPT) <@builddir@/recordingopts.ggo --file=@builddir@/recordingopts --unamed-opts=STATE
	$(CC) -c -o @builddir@/recordingopts.o @builddir@/recordingopts.c


@builddir@/recordingopts.ggo: recordingopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/recordingopts.ggi  |tr -d "\\\\" >@builddir@/recordingopts.ggo



programdir_SOURCES=programdir.cpp programdiropts.h tracker.cpp
programdir_DEPENDENCIES=programdiropts.o
programdir_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
programdir_LDADD=programdiropts.o
programdir_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

programdiropts.h: programdiropts.o



programdiropts.o: @builddir@/programdiropts.ggo
	$(GENGETOPT) <@builddir@/programdiropts.ggo --file=@builddir@/programdiropts --unamed-opts=PATH
	$(CC) -c -o @builddir@/programdiropts.o @builddir@/programdiropts.c


@builddir@/programdiropts.ggo: programdiropts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/programdiropts.ggi  |tr -d "\\\\" >@builddir@/programdiropts.ggo

runnum_SOURCES=runnum.cpp runnumopts.h tracker.cpp
runnum_DEPENDENCIES=runnumopts.o
runnum_CPPFLAGS=$(COMMON_CPPFLAGS) -I@builddir@
runnum_LDADD=runnumopts.o
runnum_LDFLAGS=libstateclient.la $(COMMON_LOADFLAGS) 

runnumopts.h: runnumopts.o



runnumopts.o: @builddir@/runnumopts.ggo
	$(GENGETOPT) <@builddir@/runnumopts.ggo --file=@builddir@/runnumopts --unamed-opts=NUMBER
	$(CC) -c -o @builddir@/runnumopts.o @builddir@/runnumopts.c


@builddir@/runnumopts.ggo: runnumopts.ggi standardopts.ggo transitionopts.ggi
	$(PREPROCESSOR) -P @srcdir@/runnumopts.ggi  |tr -d "\\\\" >@builddir@/runnumopts.ggo


# TCL Packages:
#
TCL_PACKAGES = rdoCalloutPkg.tcl

##
# Python module and sources
#

PYTHON_MODULES=RunControl.py StateControllerWidget.py StateMonitorWidget.py ReadoutGui.py



EXTRA_DIST+=$(PYTHON_MODULES) 

#  Post installation tasks:

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs/stateclient
	rm -f           @prefix@/TclLibs/stateclient/libTclstateclient.so
	$(LN_S)         @libdir@/libTclstateclient.so @prefix@/TclLibs/stateclient/libTclstateclient.so
	$(INSTALL_SCRIPT) $(TCL_PACKAGES) @prefix@/TclLibs/stateclient
	echo pkg_mkIndex -verbose @prefix@/TclLibs/stateclient "*.so" "*.tcl" | @TCLSH_CMD@

install-exec-hook:
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/rctl
	$(INSTALL_PROGRAM) @libdir@/libPyStateClient.so  @prefix@/pythonLibs/nscldaq/vardb/stateclient.so
	$(INSTALL_PROGRAM) @libdir@/libPyStateManager.so  @prefix@/pythonLibs/nscldaq/vardb/statemanager.so
	for f in $(PYTHON_MODULES); do $(INSTALL_PROGRAM) @srcdir@/$$f @prefix@/pythonLibs/nscldaq/rctl; done
	touch @prefix@/pythonLibs/nscldaq/rctl/__init__.py
	echo "#! @PYTHON@"  > @bindir@/ReadoutGui
	echo from nscldaq.rctl import ReadoutGui >> @bindir@/ReadoutGui
	echo "ReadoutGui.main()" >> @bindir@/ReadoutGui
	chmod a+x @bindir@/ReadoutGui

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


EXTRA_DIST+= stateclient.test statemgr.test  StateClientTests.py testBase.py StateManagerTests.py \
	rdoco.test RGPGuiTests.test
EXTRA_DIST+=$(TCL_PACKAGES)
EXTRA_DIST+= stateclient.xml statemanager.xml

# Test setup and specification.

TEST_EXTENSIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH; \
	DAQBIN=@prefix@/bin; export DAQBIN; DAQROOT=@prefix@; export DAQROOT; \
	DAQLIB=@prefix@/lib; export DAQLIB;
TESTS = unittests stateTests \
	stateclient.test statemgr.test \
	StateClientTests.py StateManagerTests.py rdoco.test RGPGuiTests.test

#--------------- The ring library....

lib_LTLIBRARIES = libDataFlow.la \
		libRingBuffer.la

libDataFlow_la_SOURCES = CRingBuffer.cpp CRingMaster.cpp CRemoteAccess.cpp
include_HEADERS        = CRingBuffer.h CRingMaster.h CRemoteAccess.h

noinst_HEADERS         = ringbufint.h Asserts.h testcommon.h CRingCommand.h

INCLUDES = -I@top_srcdir@/base/exception \
	   -I@top_srcdir@/base/headers 	\
	   -I@top_srcdir@/servers/portmanager \
	   -I@top_srcdir@/base/tclplus	\
	   -I@top_srcdir@/base/uri	\
		$(TCL_FLAGS)		\
	-DBINDIR='"@prefix@/bin"'

libDataFlow_la_LDFLAGS =  -version-info $(SOVERSION) \
	                  -Wl,"-rpath-link=$(libdir)" -lrt
libDataFlow_la_LIBADD  = @top_srcdir@/base/exception/libException.la \
			@top_srcdir@/servers/portmanager/libPortManager.la \
			@top_srcdir@/base/uri/liburl.la



libRingBuffer_la_SOURCES = ringPackage.cpp CRingCommand.cpp
libRingBuffer_la_DEPENDENCIES = libDataFlow.la

libRingBuffer_la_LIBADD  = 	@top_srcdir@/base/tclplus/libtclPlus.la \
				@top_srcdir@/base/dataflow/libDataFlow.la \
				@top_srcdir@/base/exception/libException.la

libRingBuffer_la_LDFLAGS = 	$(TCL_LDFLAGS) \
				-version-info $(SOVERSION) \
				-Wl,"-rpath-link=$(libdir)"


#---------------- Compiled programs. 

bin_PROGRAMS 	= ringtostdout stdintoring

ringtostdout_SOURCES = ringtostdout.cpp

ringtostdout_DEPENDENCIES = ringtostdoutsw.o libRingBuffer.la

ringtostdoutsw.o: ringtostdoutsw.ggo
	$(GENGETOPT) <ringtostdoutsw.ggo --file=ringtostdoutsw --unamed-opts
	$(COMPILE) -c ringtostdoutsw.c

ringtostdout_LDADD	= ringtostdoutsw.o libRingBuffer.la

stdintoring_SOURCES = stdintoring.cpp
stdintoring_DEPENDENCIES = stdintoringsw.o libRingBuffer.la

stdintoringsw.o: stdintoringsw.ggo
	$(GENGETOPT) <stdintoringsw.ggo --file=stdintoringsw --unamed-opts
	$(COMPILE) -c stdintoringsw.c

stdintoring_LDADD	= stdintoringsw.o libRingBuffer.la



#---------------- Tests


noinst_PROGRAMS = unittests producer consumer

unittests_SOURCES = TestRunner.cpp StaticTests.cpp TransferTests.cpp testcommon.cpp \
		DifferenceTests.cpp BlockingTests.cpp InfoTests.cpp \
		ManageTest.cpp WhilePredTest.cpp crmastertests.cpp RemoteTests.cpp

unittests_LDADD   = -L@prefix@/lib $(CPPUNIT_LDFLAGS) -lDataFlow 

unittests_LDFLAGS = -Wl,"-rpath-link=$(libdir)"

producer_LDADD    = -L@prefix@/lib -lDataFlow
producer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

consumer_LDADD    = -L@prefix@/lib -lDataFlow
consumer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests

EXTRA_DIST=ringprimitives.xml ringbuffer.xml ringlib.xml ringmaster.xml ringpipes.xml tclring.xml \
	RingMaster.tcl ringbuffer.tcl \
	ringtostdoutsw.ggo stdintoringsw.ggo 

#-------------- My installation... includes tcl scripts/packages.

install-exec-local: 
	$(INSTALL_SCRIPT) RingMaster.tcl @bindir@/RingMaster
	$(mkinstalldirs)  @prefix@/TclLibs
	$(INSTALL_SCRIPT) .libs/libRingBuffer.* @prefix@/TclLibs
	$(INSTALL_SCRIPT) ringbuffer.tcl @bindir@/ringbuffer


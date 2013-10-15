#--------------- The ring library....

lib_LTLIBRARIES = libDataFlow.la \
		libRingBuffer.la

libDataFlow_la_SOURCES = CRingBuffer.cpp  CRemoteAccess.cpp CRingMaster.cpp
include_HEADERS        = CRingBuffer.h CRingMaster.h CRemoteAccess.h

noinst_HEADERS         = ringbufint.h Asserts.h testcommon.h CRingCommand.h

INCLUDES =  -I@top_srcdir@/base/headers 	\
	   -I@top_srcdir@/servers/portmanager \
		@LIBTCLPLUS_CFLAGS@	\
	   -I@top_srcdir@/base/uri	\
	   -I@top_srcdir@/base/os	\
		$(TCL_FLAGS)		\
	-DBINDIR='"@prefix@/bin"'

libDataFlow_la_LDFLAGS =  -version-info $(SOVERSION) \
	                  -Wl,"-rpath-link=$(libdir)" 	\
			-lrt $(THREADLD_FLAGS)

libDataFlow_la_LIBADD  = @top_builddir@/servers/portmanager/libPortManager.la \
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/base/os/libdaqshm.la	\
			$(THREADLD_FLAGS)

libDataFlow_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

libRingBuffer_la_SOURCES = ringPackage.cpp CRingCommand.cpp
libRingBuffer_la_DEPENDENCIES = libDataFlow.la

libRingBuffer_la_LIBADD  = 	@top_builddir@/base/dataflow/libDataFlow.la 

libRingBuffer_la_LDFLAGS = 	@LIBEXCEPTION_LDFLAGS@ $(TCL_LDFLAGS) \
				-version-info $(SOVERSION) \
				-Wl,"-rpath-link=$(libdir)"	\
				$(THREADLD_FLAGS)

libRingBuffer_la_CXXFLAGS	=	$(THREADCXX_FLAGS) \
					$(AM_CXXFLAGS)




#---------------- Compiled programs. 

bin_PROGRAMS 	= ringtostdout stdintoring

ringtostdout_SOURCES = ringtostdout.cpp  @srcdir@/ringtostdoutsw.c

ringtostdout_DEPENDENCIES =  libDataFlow.la @srcdir@/ringtostdoutsw.h

BUILT_SOURCES=@srcdir@/ringtostdoutsw.c @srcdir@/stdintoringsw.c

@srcdir@/ringtostdoutsw.c: ringtostdoutsw.h

ringtostdoutsw.h: ringtostdoutsw.ggo
	@GENGETOPT@ <@srcdir@/ringtostdoutsw.ggo \
		--output-dir=@srcdir@ --file ringtostdoutsw --unamed-opts

@srcdir@/stdintoringsw.c: stdintoringsw.h

stdintoringsw.h: stdintoringsw.ggo
	@GENGETOPT@ <@srcdir@/stdintoringsw.ggo \
		--output-dir=@srcdir@ --file=stdintoringsw --unamed-opts

ringtostdout_LDADD	=  libDataFlow.la \
			@LIBEXCEPTION_LDFLAGS@	\
			@top_builddir@/base/os/libdaqshm.la	\
			$(THREADLD_FLAGS)

ringtostdout_CXXFLAGS	=  $(AM_CXXFLAGS) $(THREADCXX_FLAGS)

stdintoring_SOURCES = stdintoring.cpp @srcdir@/stdintoringsw.c

stdintoring_DEPENDENCIES = libDataFlow.la @srcdir@/stdintoringsw.h



stdintoring_LDADD	=  libDataFlow.la	\
			@LIBEXCEPTION_LDFLAGS@ \
			$(THREADLD_FLAGS)

stdintoring_CXXFLAGS =	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

CLEANFILES=stdintoringsw.c stdintoringsw.h ringtostdoutsw.c ringtostdoutsw.h

#---------------- Tests


noinst_PROGRAMS = unittests producer consumer

unittests_SOURCES = TestRunner.cpp StaticTests.cpp TransferTests.cpp testcommon.cpp \
		DifferenceTests.cpp BlockingTests.cpp InfoTests.cpp \
		ManageTest.cpp WhilePredTest.cpp crmastertests.cpp RemoteTests.cpp

unittests_LDADD   = -L@prefix@/lib $(CPPUNIT_LDFLAGS) \
			@LIBEXCEPTION_LDFLAGS@	\
			-lDataFlow 

unittests_LDFLAGS = -Wl,"-rpath-link=$(libdir)"


producer_SOURCES	=	producer.cpp

producer_LDADD    = -L@prefix@/lib -lDataFlow @LIBEXCEPTION_LDFLAGS@

producer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

consumer_SOURCES	=	consumer.cpp

consumer_LDADD    = -L@prefix@/lib -lDataFlow @LIBEXCEPTION_LDFLAGS@

consumer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests

EXTRA_DIST=ringprimitives.xml ringbuffer.xml ringlib.xml ringmaster.xml ringpipes.xml tclring.xml \
	RingMaster.tcl ringbuffer.tcl \
	ringtostdoutsw.ggo stdintoringsw.ggo 

#-------------- My installation... includes tcl scripts/packages.

install-exec-local: 
	$(INSTALL_SCRIPT) @srcdir@/RingMaster.tcl @bindir@/RingMaster
	$(mkinstalldirs)  @prefix@/TclLibs
	$(INSTALL_SCRIPT) @builddir@/.libs/libRingBuffer.* @prefix@/TclLibs
	$(INSTALL_SCRIPT) @srcdir@/ringbuffer.tcl @bindir@/ringbuffer


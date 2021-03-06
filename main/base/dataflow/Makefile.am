#--------------- The ring library....

lib_LTLIBRARIES = libDataFlow.la \
		libRingBuffer.la

libDataFlow_la_SOURCES = CRingBuffer.cpp CTestRingBuffer.cpp CRemoteAccess.cpp CRingMaster.cpp
include_HEADERS        = CRingBuffer.h CTestRingBuffer.h CRingMaster.h CRemoteAccess.h

noinst_HEADERS         = ringbufint.h Asserts.h testcommon.h CRingCommand.h

COMPILATION_FLAGS =  -I@top_srcdir@/base/headers 	\
	   -I@top_srcdir@/servers/portmanager \
	   -I@top_srcdir@/base/uri	\
	   -I@top_srcdir@/base/os	\
		@LIBTCLPLUS_CFLAGS@	\
		$(TCL_FLAGS)		\
	-DBINDIR='"@prefix@/bin"'

libDataFlow_la_LDFLAGS =  -version-info $(SOVERSION) \
	                  -Wl,"-rpath-link=$(libdir)" 	\
			-lrt $(THREADLD_FLAGS)

libDataFlow_la_LIBADD  = @top_builddir@/servers/portmanager/libPortManager.la \
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/base/os/libdaqshm.la	\
			$(THREADLD_FLAGS)


libDataFlow_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS) $(COMPILATION_FLAGS)

libRingBuffer_la_SOURCES = ringPackage.cpp CRingCommand.cpp
libRingBuffer_la_DEPENDENCIES = libDataFlow.la

libRingBuffer_la_LIBADD  = 	@builddir@/libDataFlow.la 

libRingBuffer_la_LDFLAGS = 	@LIBTCLPLUS_LDFLAGS@ $(TCL_LDFLAGS) \
				-version-info $(SOVERSION) \
				-Wl,"-rpath-link=$(libdir)"	\
				$(THREADLD_FLAGS) -lrt

libRingBuffer_la_CXXFLAGS	=	$(THREADCXX_FLAGS) \
					$(AM_CXXFLAGS) $(COMPILATION_FLAGS)




#---------------- Compiled programs. 

bin_PROGRAMS 	= ringtostdout stdintoring

ringtostdout_SOURCES = ringtostdout.cpp  
nodist_ringtostdout_SOURCES = ringtostdoutsw.c ringtostdoutsw.h

ringtostdout_CPPFLAGS=$(COMPILATION_FLAGS)

ringtostdout_DEPENDENCIES =  libDataFlow.la 

BUILT_SOURCES=ringtostdoutsw.c ringtostdoutsw.h  stdintoringsw.c stdintoringsw.h

ringtostdoutsw.c: ringtostdoutsw.h

ringtostdoutsw.h: ringtostdoutsw.ggo
	@GENGETOPT@ <@srcdir@/ringtostdoutsw.ggo \
		--output-dir=@builddir@ --file ringtostdoutsw --unamed-opts

stdintoringsw.c: stdintoringsw.h

stdintoringsw.h: stdintoringsw.ggo
	@GENGETOPT@ <@srcdir@/stdintoringsw.ggo \
		--output-dir=@builddir@ --file=stdintoringsw --unamed-opts

ringtostdout_LDADD	=  @builddir@/libDataFlow.la \
			@LIBEXCEPTION_LDFLAGS@	\
			@top_builddir@/base/os/libdaqshm.la	\
			$(THREADLD_FLAGS) -lrt

ringtostdout_CXXFLAGS	=  $(AM_CXXFLAGS) $(THREADCXX_FLAGS)

stdintoring_SOURCES = stdintoring.cpp 
nodist_stdintoring_SOURCES=stdintoringsw.c stdinringtosw.h

stdintoring_DEPENDENCIES = libDataFlow.la 



stdintoring_LDADD	=  @builddir@/libDataFlow.la	\
			@top_builddir@/base/os/libdaqshm.la	\
			@LIBEXCEPTION_LDFLAGS@ \
			$(THREADLD_FLAGS) -lrt

stdintoring_CXXFLAGS =	$(THREADCXX_FLAGS) $(AM_CXXFLAGS) $(COMPILATION_FLAGS)

#   CLEANFILES=stdintoringsw.c stdintoringsw.h ringtostdoutsw.c ringtostdoutsw.h

#---------------- Tests


noinst_PROGRAMS = unittests producer consumer

unittests_SOURCES = TestRunner.cpp StaticTests.cpp TransferTests.cpp testcommon.cpp \
		DifferenceTests.cpp BlockingTests.cpp InfoTests.cpp \
		ManageTest.cpp WhilePredTest.cpp crmastertests.cpp RemoteTests.cpp

unittests_LDADD   = -L@prefix@/lib $(CPPUNIT_LDFLAGS) \
			@builddir@/libDataFlow.la		\
			@top_builddir@/base/os/libdaqshm.la	\
			@LIBEXCEPTION_LDFLAGS@	\
			-lrt

unittests_LDFLAGS = -Wl,"-rpath-link=$(libdir)"

unittests_CPPFLAGS=$(COMPILATION_FLAGS)

producer_SOURCES	=	producer.cpp

producer_LDADD    = -L@prefix@/lib @builddir@/libDataFlow.la @top_builddir@/base/os/libdaqshm.la @LIBEXCEPTION_LDFLAGS@ -lrt

producer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

producer_CPPFLAGS=$(COMPILATION_FLAGS)

consumer_SOURCES	=	consumer.cpp

consumer_LDADD    = -L@prefix@/lib @builddir@/libDataFlow.la @top_builddir@/base/os/libdaqshm.la @LIBEXCEPTION_LDFLAGS@ -lrt

consumer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

consuerm_CPPFLAGS=$(COMPILATION_FLAGS)

TESTS=./unittests

EXTRA_DIST=ringprimitives.xml ringbuffer_user.xml ringbuffer_man.xml ringlib.xml ringmaster.xml ringpipes.xml tclring.xml \
	RingMaster.tcl ringbuffer.tcl \
	ringtostdoutsw.ggo stdintoringsw.ggo 

#-------------- My installation... includes tcl scripts/packages.

install-exec-local: 
	$(INSTALL_SCRIPT) @srcdir@/RingMaster.tcl @bindir@/RingMaster
	$(mkinstalldirs)  @prefix@/TclLibs
	$(INSTALL_SCRIPT) @builddir@/.libs/libRingBuffer.* @prefix@/TclLibs
	echo "pkg_mkIndex -verbose @prefix@/TclLibs *.so" | $(TCLSH_CMD)
	$(INSTALL_SCRIPT) @srcdir@/ringbuffer.tcl @bindir@/ringbuffer


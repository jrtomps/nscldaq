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
	                  -Wl,"-rpath-link=$(libdir)" -lrt	\
			$(THREADLD_FLAGS)

libDataFlow_la_LIBADD  = @top_srcdir@/base/exception/libException.la \
			@top_srcdir@/servers/portmanager/libPortManager.la \
			@top_srcdir@/base/uri/liburl.la 	\
			$(THREADLD_FLAGS)

libDataFlow_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

libRingBuffer_la_SOURCES = ringPackage.cpp CRingCommand.cpp
libRingBuffer_la_DEPENDENCIES = libDataFlow.la ringtostdoutsw.o stdintoringsw.o

libRingBuffer_la_LIBADD  = 	@top_srcdir@/base/tclplus/libtclPlus.la \
				@top_srcdir@/base/dataflow/libDataFlow.la \
				@top_srcdir@/base/exception/libException.la

libRingBuffer_la_LDFLAGS = 	$(TCL_LDFLAGS) \
				-version-info $(SOVERSION) \
				-Wl,"-rpath-link=$(libdir)"	\
				$(THREADLD_FLAGS)

libRingBuffer_la_CXXFLAGS	=	$(THREADCXX_FLAGS) \
					$(AM_CXXFLAGS)




#---------------- Compiled programs. 

bin_PROGRAMS 	= ringtostdout stdintoring

ringtostdout_SOURCES = ringtostdout.cpp

ringtostdout_DEPENDENCIES =  libRingBuffer.la


ringtostdoutsw.o: ringtostdoutsw.ggo
	$(GENGETOPT) <ringtostdoutsw.ggo --file=ringtostdoutsw --unamed-opts
	$(CC) -c ringtostdoutsw.c $(THREADC_FLAGS)

ringtostdout_LDADD	= ringtostdoutsw.o libRingBuffer.la \
			$(THREADLD_FLAGS)

ringtostdout_CXXFLAGS	= $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

stdintoring_SOURCES = stdintoring.cpp
stdintoring_DEPENDENCIES = libRingBuffer.la 

stdintoringsw.o: stdintoringsw.ggo
	$(GENGETOPT) <stdintoringsw.ggo --file=stdintoringsw --unamed-opts
	$(CC) -c stdintoringsw.c $(THREADC_FLAGS)

stdintoring_LDADD	= stdintoringsw.o libRingBuffer.la	\
			$(THREADLD_FLAGS)

stdintoring_CXXFLAGS =	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)



#---------------- Tests


noinst_PROGRAMS = unittests producer consumer

unittests_SOURCES = TestRunner.cpp StaticTests.cpp TransferTests.cpp testcommon.cpp \
		DifferenceTests.cpp BlockingTests.cpp InfoTests.cpp \
		ManageTest.cpp WhilePredTest.cpp crmastertests.cpp RemoteTests.cpp

unittests_LDADD   = -L@prefix@/lib $(CPPUNIT_LDFLAGS) -lDataFlow 

unittests_LDFLAGS = -Wl,"-rpath-link=$(libdir)"


producer_SOURCES	=	producer.cpp
producer_LDADD    = -L@prefix@/lib -lDataFlow
producer_LDFLAGS  = -Wl,"-rpath-link=$(libdir)"

consumer_SOURCES	=	consumer.cpp
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


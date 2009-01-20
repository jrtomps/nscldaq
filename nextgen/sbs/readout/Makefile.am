lib_LTLIBRARIES	=	libSBSProductionReadout.la



libSBSProductionReadout_la_SOURCES	=				\
						CReadoutMain.cpp		\
						CTCLAuthorizer.cpp		\
						CAuthorizedTclServer.cpp	\
						CExperiment.cpp			\
						RunState.cpp			\
						CComposite.cpp			\
						CScaler.cpp			\
						CEventSegment.cpp		\
						CScalerBank.cpp			\
						CCompoundEventSegment.cpp	\
						CEventTrigger.cpp		\
						CNullTrigger.cpp		\
						CTimedTrigger.cpp		\
						CCAENV262Trigger.cpp		\
						CV977Trigger.cpp		\
						CTriggerLoop.cpp


noinst_HEADERS	=	CReadoutMain.h Skeleton.h CTCLAuthorizer.h	\
			CAuthorizedTclServer.h	CExperiment.h Asserts.h	\
			RunState.h CObject.h CComposite.h CScaler.h	\
			CEventSegment.h CScalerBank.h CCompoundEventSegment.h	\
			CEventTrigger.h CNullTrigger.h CTimedTrigger.h	\
			CCAENV262Trigger.h CV977Trigger.h 	\
			CTriggerLoop.h





INCLUDES	=	-I@top_srcdir@/sbs/vmemodules	\
			-I@top_srcdir@/sbs/nsclapi	\
			-I@top_srcdir@/base/exception	\
			-I@top_srcdir@/base/tclplus	\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/servers/portmanager	\
			-I@top_srcdir@/base/dataflow		\
			-I@top_srcdir@/daq/format		\
			-I@top_srcdir@/base/thread \
			$(TCL_FLAGS) $(THREADCXX_FLAGS)	\
			-DHAVE_VME_MAPPING -DHAVE_SBSVME_INTERFACE

libSBSProductionReadout_la_LIBADD	=	@top_srcdir@/sbs/vmemodules/libSBSDeviceSupport.la	\
						@top_srcdir@/sbs/nsclapi/libSBSVmeAPI.la		\
						@top_srcdir@/daq/format/libdataformat.la		\
						@top_srcdir@/base/exception/libException.la	\
						@top_srcdir@/base/tclplus/libtclPlus.la		\
						@top_srcdir@/servers/portmanager/libPortManager.la \
						@top_srcdir@/base/dataflow/libDataFlow.la	\
						@top_srcdir@/sbs/driver/src/libbtp.la \
						options.o


libSBSProductionReadout_la_LDFLAGS	=	-version-info	1:0:0	\
						$(TCL_LDFLAGS) $(THREADLD_FLAGS)

EXTRA_DIST	=	options.ggo Skeleton.cpp

noinst_PROGRAMS	= Readout tests triggertests

Readout_SOURCES = Skeleton.cpp

Readout_DEPENDENCIES = libSBSProductionReadout.la
Readout_LDADD	= libSBSProductionReadout.la	\
		  @top_srcdir@/base/exception/libException.la	\
		  @top_srcdir@/base/thread/libdaqthreads.la \
		  @top_srcdir@/base/tclplus/libtclPlus.la $(THREADLD_FLAGS)

CLEANFILES = options.c

BUILT_SOURCES = options.o

options.o:	options.c
	$(CC) -c -I. options.c

options.c:	options.ggo
	$(GENGETOPT) <options.ggo --file=options

clean-local:
	-rm -f options.c options.h

tests_SOURCES=TestRunner.cpp experimentTests.cpp \
		compositeTests.cpp scalerbankTests.cpp	\
		compoundevsegTest.cpp			\
		triggerresponseTests.cpp 	\
		timedtriggerTest.cpp


tests_LDADD = 	CExperiment.lo	\
		CComposite.lo	\
		CScaler.lo CScalerBank.lo	\
		CEventSegment.lo CCompoundEventSegment.lo \
		RunState.lo	\
		CEventTrigger.lo CTimedTrigger.lo	\
		@top_srcdir@/daq/format/libdataformat.la	\
		@top_srcdir@/base/dataflow/libDataFlow.la	\
		@top_srcdir@/base/exception/libException.la	\
		$(CPPUNIT_LDFLAGS)  $(THREADLD_FLAGS)

tests_LDFLAGS=-Wl,"-rpath-link=$(libdir)"


triggertests_SOURCES = TestRunner.cpp CTriggerLoopTests.cpp

triggertests_LDADD = 	CTriggerLoop.lo	CEventTrigger.lo		\
			@top_srcdir@/base/thread/libdaqthreads.la \
			@top_srcdir@/base/exception/libException.la \
			$(CPPUNIT_LDFLAGS) $(THREADLD_FLAGS)

triggertests_LDFLAGS = -Wl,"-rpath-link=$(libdir)"



TESTS=./tests ./triggertests

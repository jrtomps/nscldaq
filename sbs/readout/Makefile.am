lib_LTLIBRARIES	=	libSBSProductionReadout.la


BUILT_SOURCES=options.c options.h

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
						CTriggerLoop.cpp		\
						CRunControlPackage.cpp		\
						options.c			\
						CBeginCommand.cpp		\
						CPauseCommand.cpp		\
						CResumeCommand.cpp		\
						CEndCommand.cpp			\
						CDocumentedPacket.cpp		\
						CDocumentedPacketManager.cpp	\
						CReadoutException.cpp	\
						CInvalidPacketStateException.cpp \
						CEventPacket.cpp 	\
						CVarList.cpp	\
						CDocumentedVars.cpp		\
						CVariableBuffers.cpp		\
						CBusy.cpp			\
						CCAENV262Busy.cpp		\
						CV977Busy.cpp


noinst_HEADERS	=	CReadoutMain.h Skeleton.h CTCLAuthorizer.h	\
			CAuthorizedTclServer.h	CExperiment.h Asserts.h	\
			RunState.h CObject.h CComposite.h CScaler.h	\
			CEventSegment.h CScalerBank.h CCompoundEventSegment.h	\
			CEventTrigger.h CNullTrigger.h CTimedTrigger.h	\
			CCAENV262Trigger.h CV977Trigger.h 	\
			CTriggerLoop.h CRunControlPackage.h options.h	\
			CBeginCommand.h CPauseCommand.h CResumeCommand.h	\
			CEndCommand.h CDocumentedPacket.h CDocumentedPacketManager.h \
			CReadoutException.h CInvalidPacketStateException.h	\
			CEventPacket.h CVarList.h CDocumentedVars.h CVariableBuffers.h \
			CBusy.h CCAENV262Busy.h CV977Busy.h



INCLUDES	=	-I@top_srcdir@/sbs/vmemodules	\
			-I@top_srcdir@/sbs/nsclapi	\
			@LIBTCLPLUS_CFLAGS@	\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/servers/portmanager	\
			-I@top_srcdir@/base/dataflow		\
			-I@top_srcdir@/daq/format		\
			-I@top_srcdir@/base/thread 		\
                        -I@top_srcdir@/daq/eventbuilder         \
			-I@top_srcdir@/base/os			\
			$(TCL_FLAGS) $(THREADCXX_FLAGS)	\
			-DHAVE_VME_MAPPING -DHAVE_SBSVME_INTERFACE

libSBSProductionReadout_la_LIBADD	=	@top_builddir@/sbs/vmemodules/libSBSDeviceSupport.la	\
						@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
						@top_builddir@/daq/format/libdataformat.la		\
						@top_builddir@/servers/portmanager/libPortManager.la \
						@top_builddir@/base/dataflow/libDataFlow.la	\
						@top_builddir@/sbs/				\
						@top_builddir@/base/os/libdaqshm.la			\
						@top_builddir@/sbs/driver/src/libbtp.la


libSBSProductionReadout_la_LDFLAGS	=	-version-info	1:0:0	\
						@LIBTCLPLUS_LDFLAGS@ $(TCL_LDFLAGS) $(THREADLD_FLAGS)

EXTRA_DIST	=	options.ggo Skeleton.cpp UserMakefile.in SBSRdoMakeIncludes.in SBSReadout.xml


noinst_PROGRAMS	= Readout tests

Readout_SOURCES = Skeleton.cpp

Readout_DEPENDENCIES = libSBSProductionReadout.la
Readout_LDADD	= libSBSProductionReadout.la	\
		@top_builddir@/sbs/vmemodules/libSBSDeviceSupport.la \
		  @top_builddir@/base/thread/libdaqthreads.la \
		  $(THREADLD_FLAGS)

Readout_LDFLAGS = @LIBTCLPLUS_LDFLAGS@

CLEANFILES = options.c options.h

install-exec-local:
	$(mkinstalldirs) @prefix@/skeletons/sbs
	$(mkinstalldirs) @prefix@/etc
	$(mkinstalldirs) @prefix@/include/sbsreadout
	$(INSTALL_SCRIPT) @srcdir@/Skeleton.cpp @prefix@/skeletons/sbs
	$(INSTALL_SCRIPT) @srcdir@/Skeleton.h @prefix@/skeletons/sbs
	$(INSTALL_SCRIPT) UserMakefile @prefix@/skeletons/sbs/Makefile
	$(INSTALL_SCRIPT) SBSRdoMakeIncludes @prefix@/etc
	$(INSTALL_SCRIPT) @srcdir@/*.h @prefix@/include/sbsreadout


options.c:	options.ggo
	$(GENGETOPT) <@top_srcdir@/sbs/readout/options.ggo --file=options \
		--output-dir=@top_srcdir@/sbs/readout


options.h:	options.ggo
	$(GENGETOPT) < @top_srcdir@/sbs/readout/options.ggo --file=options \
		--output-dir=@top_srcdir@/sbs/readout

clean-local:
	-rm -f options.c options.h

tests_SOURCES=TestRunner.cpp experimentTests.cpp \
		compositeTests.cpp scalerbankTests.cpp	\
		compoundevsegTest.cpp			\
		triggerresponseTests.cpp 	\
		timedtriggerTest.cpp		\
		testRunStatePkg.cpp		\
		packettests.cpp			\
		eventpackettests.cpp		\
		varlisttest.cpp docvartests.cpp	\
		testVariableBuffers.cpp	


tests_LDADD = 	CExperiment.lo	\
		CComposite.lo	\
		CScaler.lo CScalerBank.lo	\
		CEventSegment.lo CCompoundEventSegment.lo \
		RunState.lo	\
		CEventTrigger.lo CTimedTrigger.lo	\
		CNullTrigger.lo	CRunControlPackage.lo		\
		CBeginCommand.lo				\
		CPauseCommand.lo				\
		CResumeCommand.lo				\
		CEndCommand.lo					\
		CTriggerLoop.lo					\
		CDocumentedPacket.lo CDocumentedPacketManager.lo \
		CReadoutException.lo CInvalidPacketStateException.lo \
		CEventPacket.lo					\
		CDocumentedVars.lo				\
		CVarList.lo					\
		CVariableBuffers.lo				\
		@top_builddir@/daq/format/libdataformat.la	\
		@top_builddir@/base/dataflow/libDataFlow.la	\
		@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la	\
		@top_builddir@/base/thread/libdaqthreads.la	\
		@top_builddir@/base/os/libdaqshm.la		\
		$(TCL_LDFLAGS) 					\
		$(CPPUNIT_LDFLAGS)  $(THREADLD_FLAGS)

tests_LDFLAGS= @LIBTCLPLUS_LDFLAGS@ -Wl,"-rpath-link=$(libdir)"




# TESTS=./tests

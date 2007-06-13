#
#  All the sources for the directly buffered HP readout
#  are in the directory up one notch.
#


lib_LTLIBRARIES=libPReadout.la

libPReadout_la_SOURCES = \
	../AccessListCheck.cpp \
	../CBadTransitionException.cpp \
	../CBeginCommand.cpp \
	../CCompoundEventSegment.cpp \
	../CConstVariable.cpp \
	../CConstVariableCommand.cpp \
	../CDocumentedPacket.cpp \
	../CDocumentedPacketManager.cpp \
	../CEndCommand.cpp \
	../CEventSegment.cpp \
	../CExitCommand.cpp \
	../CExperiment.cpp \
	../CInterpreterCore.cpp \
	../CInterpreterShell.cpp \
	../CInvalidPacketStateException.cpp \
	../CPauseCommand.cpp \
	../CReadoutException.cpp \
	../CReadoutMain.cpp \
	../CReaper.cpp \
	../CResumeCommand.cpp \
	../CRunState.cpp \
	../CRunVariable.cpp \
	../CRunVariableBuffer.cpp \
	../CRunVariableCommand.cpp \
	../CRunVariableDumpTrigger.cpp \
	../CScalerBank.cpp \
	../CScalerTrigger.cpp \
	../CTimedScalerTrigger.cpp \
	../CStateTransitionCommand.cpp \
	../CStateVariable.cpp \
	../CStateVariableBuffer.cpp \
	../CStateVariableCommand.cpp \
	../CTCLListener.cpp \
	../CTCLServer.cpp \
	../CTagBaseCommand.cpp \
	../CTclInterpreterShell.cpp \
	../CTimedEvent.cpp \
	../CTimer.cpp \
	../CTkInterpreterShell.cpp \
	../FdInteractor.cpp \
	../HostListCheck.cpp \
	../IOInteractor.cpp \
	../Interactor.cpp \
	../PasswordCheck.cpp \
	../Skeleton.cpp \
	../StringInteractor.cpp \
	../TclAccessListCheck.cpp \
	../TclAuthorizer.cpp \
	../UnixUserCheck.cpp \
	../cmdline.c \
	../CTKListener.cpp \
	../CTKServerInstance.cpp \
	../CStrings.cpp \
	../CCAENV977Status.cpp \
	../CCAENV977Trigger.cpp \
	../CCBDCES8210ScalerTrigger.cpp \
	../CCAENV262ScalerTrigger.cpp \
	../CCAENV977ScalerTrigger.cpp


include_HEADERS = \
	../AccessListCheck.h \
	../Authenticator.h \
	../CBadTransitionException.h \
	../CBeginCommand.h \
	../CCompoundEventSegment.h \
	../CConstVariable.h \
	../CConstVariableCommand.h \
	../CDocumentedPacket.h \
	../CDocumentedPacketManager.h \
	../CEndCommand.h \
	../CEventSegment.h \
	../CExitCommand.h \
	../CExperiment.h \
	../CInterpreterCore.h \
	../CInterpreterShell.h \
	../CInvalidPacketStateException.h \
	../CPauseCommand.h \
	../CReadoutException.h \
	../CReadoutMain.h \
	../CReaper.h \
	../CResumeCommand.h \
	../CRunState.h \
	../CRunVariable.h \
	../CRunVariableBuffer.h \
	../CRunVariableCommand.h \
	../CRunVariableDumpTrigger.h \
	../CScalerBank.h    \
	../CScalerTrigger.h \
	../CTimedScalerTrigger.h \
	../CStateTransitionCommand.h \
	../CStateVariable.h \
	../CStateVariableBuffer.h \
	../CStateVariableCommand.h \
	../CTCLListener.h \
	../CTCLServer.h \
	../CTagBaseCommand.h \
	../CTclInterpreterShell.h \
	../CTimedEvent.h \
	../CTimer.h \
	../CTkInterpreterShell.h \
	../CTraditionalEventSegment.h \
	../CTraditionalScalerReadout.h \
	../FdInteractor.h \
	../HostListCheck.h \
	../IOInteractor.h \
	../Interactor.h \
	../PasswordCheck.h \
	../StringInteractor.h \
	../TclAccessListCheck.h \
	../TclAuthorizer.h \
	../UnixUserCheck.h \
	../cmdline.h  \
	../CTKServerInstance.h \
	../CTKListener.h  \
	../CStrings.h \
	../CCAENV977Status.h \
	../CCAENV977Trigger.h \
	../CCBDCES8210ScalerTrigger.h \
	../CCAENV262ScalerTrigger.h \
	../CCAENV977ScalerTrigger.h



libPReadout_la_LDFLAGS = -g -version-info $(SOVERSION):0 \
	@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
	@top_srcdir@/framework/libEventFramework.la 	\
	@top_srcdir@/OutputBuffers/libOutputBuffers.la	\
	$(VMEBUILDLIBS) \
	@top_srcdir@/Exception/libException.la	\
	-lrt  $(SPECTRODAQ_LDFLAGS) \
	$(TCL_LDFLAGS) $(X11LIBS) -Wl,"-rpath-link=$(libdir)"

INCLUDES = -I..  $(CAMAC) -I@top_srcdir@/Headers \
	-I@top_srcdir@/SpectrodaqLite	\
	$(VMEBUILDHEADERS)        \
	-I@top_srcdir@/DeviceSupport \
	-I@top_srcdir@/CopyrightTools     \
	-I@top_srcdir@/VMEApi     \
	-I@top_srcdir@/OutputBuffers	\
	-I@top_srcdir@/framework  \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL \
	$(TCL_FLAGS) $(SPECTRODAQ_FLAGS)



install-data-local:
	$(mkinstalldirs) $(prefix)/pReadoutSkeleton
	$(mkinstalldirs) $(prefix)/etc
	chmod a+rx $(prefix)
	chmod a+rx $(prefix)/pReadoutSkeleton
	$(INSTALL_DATA) ../MyExperiment.cpp $(prefix)/pReadoutSkeleton/Skeleton.cpp
	echo INSTDIR=$(prefix) > Makefile.user
	cat  Makefile.skel    >> Makefile.user
	$(INSTALL_DATA) Makefile.user $(prefix)/pReadoutSkeleton/Makefile
	$(INSTALL_DATA) ProductionReadout_Makefile.include $(prefix)/etc
	$(INSTALL_DATA) ../CTraditionalEventSegment.cpp \
		$(prefix)/pReadoutSkeleton/CTraditionalEventSegment.cpp
	$(INSTALL_DATA) ../CTraditionalScalerReadout.cpp \
	        $(prefix)/pReadoutSkeleton/CTraditionalScalerReadout.cpp

EXTRA_DIST=Makefile.skel  ProductionReadout_Makefile.include
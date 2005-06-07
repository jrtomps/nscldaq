
lib_LTLIBRARIES=libHPPreadout.la
libHPPreadout_la_SOURCES = AccessListCheck.cpp \
	AuthTokenCheck.cpp \
	CBadTransitionException.cpp \
	CBeginCommand.cpp \
	CCompoundEventSegment.cpp \
	CConstVariable.cpp \
	CConstVariableCommand.cpp \
	CDocumentedPacket.cpp \
	CDocumentedPacketManager.cpp \
	CEndCommand.cpp \
	CEventSegment.cpp \
	CExitCommand.cpp \
	CExperiment.cpp \
	CInterpreterCore.cpp \
	CInterpreterShell.cpp \
	CInvalidPacketStateException.cpp \
	CNSCLBeginBuffer.cpp \
	CNSCLControlBuffer.cpp \
	CNSCLDocumentationBuffer.cpp \
	CNSCLEndBuffer.cpp \
	CNSCLOutputBuffer.cpp \
	CNSCLPauseBuffer.cpp \
	CNSCLPhysicsBuffer.cpp \
	CNSCLResumeBuffer.cpp \
	CNSCLScalerBuffer.cpp \
	CNSCLStringListBuffer.cpp \
	CPauseCommand.cpp \
	CReadoutException.cpp \
	CReadoutMain.cpp \
	CReaper.cpp \
	CResumeCommand.cpp \
	CRunState.cpp \
	CRunVariable.cpp \
	CRunVariableBuffer.cpp \
	CRunVariableCommand.cpp \
	CRunVariableDumpTrigger.cpp \
	CScalerBank.cpp \
	CScalerTrigger.cpp \
	CStateTransitionCommand.cpp \
	CStateVariable.cpp \
	CStateVariableBuffer.cpp \
	CStateVariableCommand.cpp \
	CTCLListener.cpp \
	CTCLServer.cpp \
	CTagBaseCommand.cpp \
	CTclInterpreterShell.cpp \
	CTimedEvent.cpp \
	CTimer.cpp \
	CTkInterpreterShell.cpp \
	DAQAuthTokenInteractor.cpp \
	FdInteractor.cpp \
	HostListCheck.cpp \
	IOInteractor.cpp \
	Interactor.cpp \
	PasswordCheck.cpp \
	Skeleton.cpp \
	StringInteractor.cpp \
	TclAccessListCheck.cpp \
	TclAuthorizer.cpp \
	UnixUserCheck.cpp \
	cmdline.c \
	CTKListener.cpp CTKServerInstance.cpp \
	CStrings.cpp

noinst_HEADERS = AccessListCheck.h \
	AuthTokenCheck.h \
	Authenticator.h \
	CBadTransitionException.h \
	CBeginCommand.h \
	CCompoundEventSegment.h \
	CConstVariable.h \
	CConstVariableCommand.h \
	CDocumentedPacket.h \
	CDocumentedPacketManager.h \
	CEndCommand.h \
	CEventSegment.h \
	CExitCommand.h \
	CExperiment.h \
	CInterpreterCore.h \
	CInterpreterShell.h \
	CInvalidPacketStateException.h \
	CNSCLBeginBuffer.h \
	CNSCLControlBuffer.h \
	CNSCLDocumentationBuffer.h \
	CNSCLEndBuffer.h \
	CNSCLOutputBuffer.h \
	CNSCLPauseBuffer.h \
	CNSCLPhysicsBuffer.h \
	CNSCLResumeBuffer.h \
	CNSCLScalerBuffer.h \
	CNSCLStringListBuffer.h \
	CPauseCommand.h \
	CReadoutException.h \
	CReadoutMain.h \
	CReaper.h \
	CResumeCommand.h \
	CRunState.h \
	CRunVariable.h \
	CRunVariableBuffer.h \
	CRunVariableCommand.h \
	CRunVariableDumpTrigger.h \
	CScalerBank.h    \
	CScalerTrigger.h \
	CStateTransitionCommand.h \
	CStateVariable.h \
	CStateVariableBuffer.h \
	CStateVariableCommand.h \
	CTCLListener.h \
	CTCLServer.h \
	CTagBaseCommand.h \
	CTclInterpreterShell.h \
	CTimedEvent.h \
	CTimer.h \
	CTkInterpreterShell.h \
	CTraditionalEventSegment.h \
	CTraditionalScalerReadout.h \
	DAQAuthTokenInteractor.h \
	FdInteractor.h \
	HostListCheck.h \
	IOInteractor.h \
	Interactor.h \
	PasswordCheck.h \
	StringInteractor.h \
	TclAccessListCheck.h \
	TclAuthorizer.h \
	UnixUserCheck.h \
	cmdline.h  \
	CTKServerInstance.h \
	CTKListener.h  CStrings.h


libHPPreadout_la_LDFLAGS = -g -version-info $(SOVERSION):0 \
	-L@top_srcdir@/framework \
	-L@top_srcdir@/VMEApi \
	-L@top_srcdir@/Exception \
	-lEventFramework -lVmeAPI -lException -lrt  $(SPECTRODAQ_LDFLAGS) \
	$(TCL_LDFLAGS) $(X11LIBS) -Wl,"-rpath-link=$(libdir)"

INCLUDES =  $(CAMAC) -I@top_srcdir@/Headers \
	$(VMEBUILDHEADERS)        \
	-I@top_srcdir@/DeviceSupport \
	-I@top_srcdir@/CopyrightTools     \
	-I@top_srcdir@/VMEApi     \
	-I@top_srcdir@/framework  \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL \
	$(TCL_FLAGS) $(SPECTRODAQ_FLAGS)

EXTRA_DIST = 	MyExperiment.cpp \
	CTraditionalScalerReadout.cpp \
	CTraditionalEventSegment.cpp  \
	Makefile.skel ProductionReadout_Makefile.include


install-data-local:
	$(mkinstalldirs) $(prefix)/HPpReadoutSkeleton
	$(mkinstalldirs) $(prefix)/HPpReadoutIncludes
	$(mkinstalldirs) $(prefix)/etc
	chmod a+rx $(prefix)
	chmod a+rx $(prefix)/HPpReadoutSkeleton
	$(INSTALL_DATA) MyExperiment.cpp $(prefix)/HPpReadoutSkeleton/Skeleton.cpp
	echo INSTDIR=$(prefix) > Makefile.user
	cat  Makefile.skel    >> Makefile.user
	$(INSTALL_DATA) Makefile.user $(prefix)/HPpReadoutSkeleton/Makefile
	$(INSTALL_DATA) ProductionReadout_Makefile.include \
	      $(prefix)/etc/HPProductionReadout_Makefile.include
	$(INSTALL_DATA) CTraditionalEventSegment.cpp \
		$(prefix)/HPpReadoutSkeleton/CTraditionalEventSegment.cpp
	$(INSTALL_DATA) CTraditionalScalerReadout.cpp \
	        $(prefix)/HPpReadoutSkeleton/CTraditionalScalerReadout.cpp
	$(INSTALL_DATA) *.h $(prefix)/HPpReadoutIncludes



CXXFLAGS=-g
CCFLAGS=-g

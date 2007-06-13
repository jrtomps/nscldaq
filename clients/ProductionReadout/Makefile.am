

SUBDIRS = hyperlatex direct HP


PRODUCTIONSOURCES = AccessListCheck.cpp \
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
	CStrings.cpp \
	CCAENV977Status.cpp CCAENV977Trigger.cpp \
	CCBDCES8210ScalerTrigger.cpp             \
	CCAENV262ScalerTrigger.cpp             \
	CCAENV977ScalerTrigger.cpp


PRODUCTIONHEADERS = AccessListCheck.h \
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
	CTKListener.h  CStrings.h \
	CCAENV977Status.h CCAENV977Trigger.h \
	CCBDCES8210ScalerTrigger.h \
	CCAENV262ScalerTrigger.h   \
	CCAENV977ScalerTrigger.h


OTHER_STUFF  = 	MyExperiment.cpp \
	CTraditionalScalerReadout.cpp \
	CTraditionalEventSegment.cpp  \
	Makefile.skel ProductionReadout_Makefile.include

DOCFILES=ScalerTriggerAppnote.xml \
	CScalerTrigger.xml CCAENV262ScalerTrigger.xml   \
	CCAENV977ScalerTrigger.xml CCBDCES8210ScalerTrigger.xml \
	CTimedScalerTrigger.xml 

EXTRA_DIST=$(OTHER_STUFF) $(PRODUCTIONSOURCES) $(PRODUCTIONHEADERS) \
	$(DOCFILES)


# PDF files if made go to Docs
# Man pages are all in the man3 subdir
#
install-data-local: documentation
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/man $(prefix)/man/man3
	-$(INSTALL_DATA) ScalerTriggerAppnote.pdf $(prefix)/Docs
	-for f in *Trigger.3*; do $(INSTALL_DATA) $$f $(prefix)/man/man3; done


documentation: manpages appnote.pdf

appnote.pdf : ScalerTriggerAppnote.xml
	$(HCDOCBOOK) ScalerTriggerAppnote.xml
	if [ -e ScalerTriggerAppnote.dvi ]; then          \
		$(DVIPDF)    ScalerTriggerAppnote.dvi	; \
	else                                              \
		touch ScalerTriggerAppnote.pdf;           \
	fi

manpages: CScalerTrigger.xml CCAENV262ScalerTrigger.xml   \
	CCAENV977ScalerTrigger.xml CCBDCES8210ScalerTrigger.xml CTimedScalerTrigger.xml
	-for f in $+; do $(MANDOCBOOK) man $$f 2>/dev/null ;   done 


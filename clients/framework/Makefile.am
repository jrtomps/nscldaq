lib_LTLIBRARIES= libEventFramework.la
libEventFramework_la_SOURCES= CThreadRecursiveMutex.cpp  \
	CEventLoop.cpp CXtEventLoop.cpp \
	CApplicationSerializer.cpp  \
	CNamedObject.cpp CObjectRegistry.cpp \
	CClassifiedObjectRegistry.cpp CSocket.cpp CRegisteredObject.cpp \
	CApplicationRegistry.cpp CReactor.cpp CFdReactor.cpp \
	CLocationReactor.cpp \
	CBufferReactor.cpp CEventMonitor.cpp CBufferMonitor.cpp\
	CFdMonitor.cpp CTimerMonitor.cpp CEvent.cpp CLocationMonitor.cpp \
	CPointerPredicate.cpp CChangedPredicate.cpp \
	CMaskedValuePredicate.cpp  \
	CFileEvent.cpp CLocationEvent.cpp CBufferEvent.cpp CTimerEvent.cpp \
	CServerConnectionEvent.cpp CServerInstance.cpp \
	CVariableBinding.cpp CArrayBinding.cpp CAssocArrayBinding.cpp \
	CConfigurationManager.cpp CLogger.cpp CAlarmLogger.cpp \
	CTCPBadSocketState.cpp \
	CTCPConnectionFailed.cpp \
	CTCPConnectionLost.cpp \
	CTCPNoSuchHost.cpp \
	CTCPNoSuchService.cpp \
	CNoSuchObjectException.cpp \
	CDuplicateNameException.cpp \
	CDuplicateSingleton.cpp \
	CIncompatibleMonitor.cpp \
	XMCallback.cpp \
	XMDialogs.cpp    \
	XMManagers.cpp   \
	XMMenus.cpp      \
	XMShell.cpp      \
	XMWlist.cpp 	\
	XMHelp.cpp


include_HEADERS= CAlarmLogger.h \
	CApplicationRegistry.h \
	CApplicationSerializer.h \
	CArrayBinding.h \
	CAssocArrayBinding.h \
	CBinding.h \
	CBufferEvent.h \
	CBufferMonitor.h \
	CBufferReactor.h \
	CChangedPredicate.h \
	CClassifiedObjectRegistry.h \
	CConfigurationManager.h \
	CDAQTCLProcessor.h \
	CDuplicateNameException.h \
	CDuplicateSingleton.h \
	CEvent.h \
	CEventLoop.h \
	CEventMonitor.h \
	CFdMonitor.h \
	CFdReactor.h \
	CFileEvent.h \
	CIncompatibleMonitor.h \
	CInterpreterStartup.h \
	CLinkFailedException.h \
	CLocationEvent.h \
	CLocationMonitor.h \
	CLocationReactor.h \
	CLogger.h \
	CMaskedValuePredicate.h \
	CNamedObject.h \
	CNoSuchLinkException.h \
	CNoSuchObjectException.h \
	CObjectRegistry.h \
	CPointerPredicate.h \
	CReactor.h \
	CRefptr.h \
	CRegisteredObject.h \
	CServerConnectionEvent.h \
	CServerInstance.h \
	CServerMonitor.h \
	CSocket.h \
	CTCLInterpreterStartup.h \
	CTCLSynchronizeCommand.h \
	CTCPBadSocketState.h \
	CTCPConnectionFailed.h \
	CTCPConnectionLost.h \
	CTCPNoSuchHost.h \
	CTCPNoSuchService.h \
	CTKInterpreterStartup.h \
	CThreadRecursiveMutex.h \
	CTimerEvent.h \
	CTimerMonitor.h \
	CTypeFreeBinding.h \
	CVariableBinding.h \
	CXtEventLoop.h \
	ClocationMonitor.h \
	SpectroFramework.h \
	XMCallback.h \
	XMDialogs.h \
	XMLabel.h \
	XMList.h \
	XMManagers.h \
	XMMenubar.h \
	XMMenus.h \
	XMPushbutton.h \
	XMScale.h \
	XMSeparators.h \
	XMShell.h \
	XMText.h \
	XMWidget.h \
	XMWlist.h 

INCLUDES= -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL -I@top_srcdir@/VMEApi \
	$(SPECTRODAQ_FLAGS) $(TCL_FLAGS) $(X11INCS)

EXTRA_DIST=Doxyfile BufferEvent.oxy \
	EventDriven.oxy \
	FileEvents.oxy \
	Registration.oxy \
	ServerEvent.oxy \
	SocketEvents.oxy \
	Subclassing.oxy \
	Synchronization.oxy \
	XtGuests.oxy \
	configsys.oxy \
	locationmonitor.oxy \
	maindocs.oxy \
	tcpip.oxy \
	docexamples/XtExample.cpp   docexamples/configtest.mak  \
	docexamples/registry.cpp  docexamples/subclass.mak \
	docexamples/TclExample.cpp  docexamples/XtExample.mak \
	docexamples/locationex.cpp  docexamples/registry.mak \
	docexamples/TclExample.mak  docexamples/buffers.cpp   \
	docexamples/locationex.mak  docexamples/server.cpp    \
	docexamples/TkExample.cpp   docexamples/buffers.mak    \
	 docexamples/minimal.cpp     docexamples/server.mak \
	docexamples/TkExample.mak   docexamples/configtest.cpp  \
	docexamples/minimal.mak     docexamples/subclass.cpp

libEventFramework_la_LDFLAGS = -version-info 1:0:0
#
# Doxygen documentation.
#
install-data-local:
	$(DOXYGEN)
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/Framework
	if [ -d docs/html ] ; then \
	(cd docs/html; \
	for f in $$(ls); \
	do \
	  $(INSTALL_DATA) $$f $(prefix)/Docs/Framework; \
        done) \
 	fi

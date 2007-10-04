lib_LTLIBRARIES= libEventFramework.la
TEMPLATE_IMPLEMENTATIONS= CArrayBinding.cpp                      \
	CAssocArrayBinding.cpp	                        \
	CBufferEvent.cpp                                \
	CBufferMonitor.cpp                              \
	CBufferReactor.cpp                              \
	CVariableBinding.cpp      

libEventFramework_la_SOURCES= CThreadRecursiveMutex.cpp  \
	CEventLoop.cpp  \
	CApplicationSerializer.cpp  \
	CNamedObject.cpp CObjectRegistry.cpp \
	CClassifiedObjectRegistry.cpp CSocket.cpp CRegisteredObject.cpp \
	CApplicationRegistry.cpp CReactor.cpp CFdReactor.cpp \
	CInterpreterStartup.cpp CDAQTCLProcessor.cpp \
	CTCLSynchronizeCommand.cpp \
	CTCLInterpreterStartup.cpp CTKInterpreterStartup.cpp \
	CBufferReactor.cpp CEventMonitor.cpp CBufferMonitor.cpp\
	CFdMonitor.cpp CTimerMonitor.cpp CEvent.cpp  \
	CFileEvent.cpp  CBufferEvent.cpp CTimerEvent.cpp \
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
	CIncompatibleMonitor.cpp 

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
	CLogger.h \
	CNamedObject.h \
	CNoSuchLinkException.h \
	CNoSuchObjectException.h \
	CObjectRegistry.h \
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
	SpectroFramework.h 


INCLUDES= -I@top_srcdir@/Headers	\
	-I@top_srcdir@/SpectrodaqLite  -I@top_srcdir@/Exception \
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

libEventFramework_la_LIBADD = @top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la

libEventFramework_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"
#
# Doxygen documentation... and template class implementations
#  which must be installedin $(prefix)/include.
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
	$(mkinstalldirs) $(prefix)/include
	$(INSTALL_DATA) $(TEMPLATE_IMPLEMENTATIONS) $(prefix)/include


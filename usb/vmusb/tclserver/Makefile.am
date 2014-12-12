lib_LTLIBRARIES			= libVMUSBControlServer.la libUserDriver.la


COMPILATION_FLAGS=@THREADCXX_FLAGS@ -I@top_srcdir@/usb/vmusb/vmusb		\
					-I@top_srcdir@/usb/common/configurableobject \
					-I@top_srcdir@/usb/common/slowcontrols \
					-I@top_srcdir@/daq/format                 \
					-I@top_srcdir@/usb/vmusb/devices	\
					-I@top_srcdir@/usb/vmusb/rdothread	\
					-I@top_srcdir@/usb/vmusb/router	\
					-I@top_srcdir@/usb/vmusb	\
					@LIBTCLPLUS_CFLAGS@	\
					-I@top_srcdir@/base/headers \
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/base/os		\
					@TCL_FLAGS@

libVMUSBControlServer_la_CXXFLAGS	= $(COMPILATION_FLAGS)

libVMUSBControlServer_la_SOURCES	= \
					CModuleCommand.cpp	\
					TclServer.cpp		\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CGDG.cpp		\
					CV812.cpp		\
					CVMUSBModule.cpp	\
					CV6533.cpp		\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					CMonCommand.cpp         \
					Server.cpp		\
					CWatchCommand.cpp	\
					CModuleFactory.cpp	\
					CModuleCreator.cpp	\
					CJtecgdgCreator.cpp	\
					CV812Creator.cpp	\
					CVMUSBCreator.cpp	\
					CV6533Creator.cpp	\
					CTclModuleCreator.cpp    \
					CTclControlModule.cpp \
					CXLMControls.cpp	\
					CXLMControlsCreator.cpp \
          CRunStateCommand.cpp \
					CMxDCRCBus.cpp \
					CMxDCRCBusCreator.cpp \
					ChicoTrigger.cpp \
					CChicoTriggerCreator.cpp

noinst_HEADERS			=	CGDG.h			\
					CV812.h			\
					CV6533.h		\
					CVMUSBModule.h		\
					CSetCommand.h		\
					server.h		\
					CControlHardware.h	\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					CMonCommand.h          \
					TclServer.h		\
					CWatchCommand.h		\
					CModuleFactory.h	\
					CModuleCreator.h	\
					CJtecgdgCreator.h       \
					CV812Creator.h		\
					CVMUSBCreator.h		\
					CV6533Creator.h		\
					CTclModuleCreator.h	\
					CTclControlModule.h	\
					CXLMControls.h	\
					CXLMControlsCreator.h \
          CRunStateCommand.h \
					CMxDCRCBus.h \
					CMxDCRCBusCreator.h \
					ChicoTrigger.h \
					CChicoTriggerCreator.h



libVMUSBControlServer_la_LIBADD	= 	@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
					@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
					@top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
					@top_builddir@/usb/vmusb/rdothread/libVMUSBRdoThread.la \
					@top_builddir@/usb/vmusb/router/libVMUSBRouter.la	\
					@LIBTCLPLUS_LDFLAGS@	\
					@top_builddir@/base/os/libdaqshm.la	\
					@top_builddir@/base/thread/libdaqthreads.la \
					@TCL_LDFLAGS@ @THREADLD_FLAGS@

libUserDriver_la_SOURCES  = CUserDriverSample.cpp

libUserDriver_la_CPPFLAGS = $(COMPILATION_FLAGS)

install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(mkinstalldirs) @prefix@/vmusbdriver/example
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/CUserDriverSample.cpp @prefix@/vmusbdriver/example


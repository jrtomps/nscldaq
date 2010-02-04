lib_LTLIBRARIES			= libControlServer.la
libControlServer_la_SOURCES	= 	CControlHardware.cpp	\
					CModuleCommand.cpp	\
					TclServer.cpp		\
					CControlModule.cpp	\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					DriverTemplate.cpp	\
					Server.cpp		\
					C894.cpp

include_HEADERS			=	CSetCommand.h		\
					server.h		\
					CControlHardware.h	\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					DriverTemplate.h	\
					TclServer.h		\
					C894.h

INCLUDES			= 	-I$(top_builddir)/ccusb		\
					-I$(top_builddir)/devices	\
					-I$(top_builddir)/threadcomm	\
					-I$(top_builddir)/rdothread	\
					-I$(top_builddir)/router	\
					@TCL_FLAGS@ @NSCLDAQ_FLAGS@	\
					@SPECTRODAQ_FLAGS@

libControlServer_la_LIBADD	= 	$(top_builddir)/ccusb/libCCUSB.la          \
					$(top_builddir)/devices/libVMUSBDevices.la \
					$(top_builddir)/threadcomm/libThreadComm.la \
					$(top_builddir)/rdothread/libRdoThread.la \
					$(top_builddir)/router/libRouter.la	\
					@TCL_LDFLAGS@				\
					@NSCLDAQ_LIB@ -lTclPlus -lException	\
					@SPECTRODAQ_LDFLAGS@
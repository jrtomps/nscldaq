lib_LTLIBRARIES			= libControlServer.la
libControlServer_la_SOURCES	= 	CControlHardware.cpp	\
					CModuleCommand.cpp	\
					TclServer.cpp		\
					CControlModule.cpp	\
					CSetCommand.cpp		\
					serverinstance.cpp	\
					CGDG.cpp		\
					CV812.cpp		\
					CUpdateCommand.cpp	\
					CGetCommand.cpp		\
					Server.cpp

include_HEADERS			=	CGDG.h			\
					CV812.h			\
					CSetCommand.h		\
					server.h		\
					CControlHardware.h	\
					CGetCommand.h		\
					CUpdateCommand.h	\
					serverinstance.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					TclServer.h

INCLUDES			= 	-I$(top_builddir)/vmusb		\
					-I$(top_builddir)/devices	\
					-I$(top_builddir)/threadcomm	\
					-I$(top_builddir)/rdothread	\
					-I$(top_builddir)/router	\
					@TCL_FLAGS@ @NSCLDAQ_FLAGS@	\
					@SPECTRODAQ_FLAGS@

libControlServer_la_LIBADD	= 	$(top_builddir)/vmusb/libVMUSB.la          \
					$(top_builddir)/devices/libVMUSBDevices.la \
					$(top_builddir)/threadcomm/libThreadComm.la \
					$(top_builddir)/rdothread/libRdoThread.la \
					$(top_builddir)/router/libRouter.la	\
					@TCL_LDFLAGS@				\
					@NSCLDAQ_LIB@ -lTclPlus -lException	\
					@SPECTRODAQ_LDFLAGS@
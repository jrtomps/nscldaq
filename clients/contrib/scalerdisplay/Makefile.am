lib_LTLIBRARIES=libScalerServer.la
bin_PROGRAMS=ScalerServer
ScalerServer_DEPENDENCIES = libScalerServer.la

libScalerServer_la_LDFLAGS = -version-info $(SOVERSION):0  \
	-Wl,"-rpath-link=$(libdir)"

ScalerServer_LDFLAGS = 	-Wl,"-rpath=$(libdir)"

libScalerServer_la_SOURCES =  CBoolConfigParam.cpp \
	CCAENModule.cpp \
	CCAENV775.cpp \
	CCAENV775Creator.cpp \
	CCAENV785.cpp \
	CCAENV785Creator.cpp \
	CCAENV792.cpp \
	CCAENV792Creator.cpp \
	CCAENV830Creator.cpp \
	CCAENV830Module.cpp \
	CConfigurationParameter.cpp \
	CConfigurator.cpp \
	CDigitizerDictionary.cpp \
	CDigitizerModule.cpp \
	CIntArrayParam.cpp \
	CIntConfigParam.cpp \
	CModuleCommand.cpp \
	CModuleCreator.cpp \
	CStringArrayparam.cpp \
	CStringConfigParam.cpp \
	Server.cpp \
	TclAuthorizer.cpp \
	serverinstance.cpp \
	CPacketCreator.cpp	\
	CPacket.cpp	\
	tclAppInit.cpp 

ScalerServer_SOURCES =	tkAppInit.cpp 

noinst_HEADERS = CBoolConfigParam.h \
	CCAENModule.h \
	CCAENV775.h \
	CCAENV775Creator.h \
	CCAENV785.h \
	CCAENV785Creator.h \
	CCAENV792.h \
	CCAENV792Creator.h \
	CCAENV830Creator.h \
	CCAENV830Module.h \
	CConfigurationParameter.h \
	CConfigurator.h \
	CDigitizerDictionary.h \
	CDigitizerModule.h \
	CIntArrayParam.h \
	CIntConfigParam.h \
	CModuleCommand.h \
	CModuleCreator.h \
	CStringArrayparam.h \
	CStringConfigParam.h \
	TclAuthorizer.h \
	server.h \
	CPacketCreator.h	\
	CPacket.h	\
	serverinstance.h 

EXTRA_DIST= mwutil.tcl \
	notebook.tcl \
	scaler.tcl \
	tablelist.tcl \
	tablelistSortByColumn.tcl \
	tablelistWidget.tcl \
	tabnbook.tcl \
	Makefile.skel scalerdisplay_Makefile.include

INCLUDES= -I. -I@top_srcdir@/Headers \
	-I@top_srcdir@/TCL -I@top_srcdir@/Exception \
	 -I@top_srcdir@/CopyrightTools \
	$(TCL_FLAGS)

ScalerServer_LDADD =  -L. -lScalerServer \
	-L@top_srcdir@/TCL -lTclPlus -L@top_srcdir@/Exception \
	-L@top_srcdir@/CopyrightTools -lLicense -lException $(TCL_LDFLAGS) \
	$(X11LIBS) 


#
#   At installation time, in addition to putting the scaler display program into
#   the bin and its shared library in the lib directory, we want to:
#   - Create directories that can be used to extend the software as follows:
#   -      contrib/scalerdisplay
#   -      contrib/scalerdisplay/include  - All headers from here.
#   -      contrib/scalerdisplay/src      - All .cpp files from here.
#   -      contrib/scalerdisplay/scripts  - All *.tcl files and any other scripts.
#   -      contrib/scalerdisplay/skel     - Starting point for extension.
#
install-exec-local:
	$(mkinstalldirs)	$(prefix)/contrib
	$(mkinstalldirs)	$(prefix)/contrib/scalerdisplay
	$(mkinstalldirs)	$(prefix)/contrib/scalerdisplay/include
	$(mkinstalldirs)	$(prefix)/contrib/scalerdisplay/src
	$(mkinstalldirs)	$(prefix)/contrib/scalerdisplay/scripts
	$(mkinstalldirs)	$(prefix)/contrib/scalerdisplay/skel
	#
	#  Install the headers, sources and scripts.
	#
	$(INSTALL_DATA)  *.cpp  $(prefix)/contrib/scalerdisplay/src
	$(INSTALL_DATA)  *.h    $(prefix)/contrib/scalerdisplay/include
	$(INSTALL_DATA)  *.tcl  $(prefix)/contrib/scalerdisplay/scripts
	#
	# Build the skeleton directory:
	#
	$(INSTALL_DATA) tkAppInit.cpp $(prefix)/contrib/scalerdisplay/skel
	echo INSTDIR=$(prefix)                                 > Makefile.user
	cat  Makefile.skel                                    >> Makefile.user
	echo SCALERDISPLAYINCLUDE=$(prefix)/contrib/scalerdisplay/include \
	$(INSTALL_DATA) Makefile.user \
	       $(prefix)/contrib/scalerdisplay/skel/Makefile
	$(INSTALL_DATA) scalerdisplay_Makefile.include $(prefix)/etc




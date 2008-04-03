bin_PROGRAMS = ScriptedReadout
lib_LTLIBRARIES=libScriptedRdo.la



ScriptedReadout_SOURCES = ReadoutMain.cpp  skeleton.cpp
ScriptedReadout_DEPENDENCIES = libScriptedRdo.la


libScriptedRdo_la_SOURCES = CBoolConfigParam.cpp \
	CCAENModule.cpp \
	CCAENTrigger.cpp \
	CCAENV775.cpp \
	CCAENV775Creator.cpp \
	CCAENV785.cpp \
	CCAENV785Creator.cpp \
	CCAENV792.cpp \
	CCAENV792Creator.cpp \
	CCAENV830.cpp \
	CCAENV830Creator.cpp \
	CCAENV830Module.cpp \
	CSIS3300Creator.cpp \
	CSIS3300Module.cpp  \
	CConfigurationParameter.cpp \
	CDigitizerDictionary.cpp \
	CDigitizerModule.cpp \
	CIntArrayParam.cpp \
	CIntConfigParam.cpp \
	CModuleCommand.cpp \
	CModuleCreator.cpp \
	CNullBusy.cpp \
	CReadOrder.cpp \
	CScalerBank.cpp \
	CScalerModule.cpp \
	CStringArrayparam.cpp \
	CStringConfigParam.cpp \
	CTriggerCommand.cpp \
	CConfigurableObject.cpp \
	CReadableObject.cpp 	\
	CReadException.cpp	\
	CPacketCreator.cpp	\
	CTraditionalV977Trigger.cpp \
	CTraditionalV977Busy.cpp  \
	CEnumParameter.cpp



libScriptedRdo_la_LDFLAGS = -version-info $(SOVERSION):0 

ScriptedReadout_LDFLAGS = -Wl,"-rpath=$(prefix)/lib"


noinst_HEADERS=CBoolConfigParam.h \
	CCAENModule.h \
	CCAENTrigger.h \
	CCAENV775.h \
	CCAENV775Creator.h \
	CCAENV785.h \
	CCAENV785Creator.h \
	CCAENV792.h \
	CCAENV792Creator.h \
	CCAENV820Registers.h \
	CCAENV830.h \
	CCAENV830Creator.h \
	CCAENV830Module.h \
	CSIS3300Creator.h \
	CSIS3300Module.h \
	CConfigurationParameter.h \
	CDigitizerDictionary.h \
	CDigitizerModule.h \
	CIntArrayParam.h \
	CIntConfigParam.h \
	CModuleCommand.h \
	CModuleCreator.h \
	CNullBusy.h \
	CReadOrder.h \
	CScalerBank.h \
	CScalerModule.h \
	CStringArrayparam.h \
	CStringConfigParam.h \
	CTriggerCommand.h  \
	CConfigurableObject.h	\
	CReadableObject.h	\
	CReadException.h	\
	CPacketCreator.h       \
	CTraditionalV977Trigger.h \
	CTraditionalV977Busy.h   \
	CEnumParameter.h




INCLUDES=  $(VMEBUILDEADERS) \
	-I@top_srcdir@/SpectrodaqLite \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL \
	-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/DeviceSupport\
	-I@top_srcdir@/Readout \
	-DVME $(TCL_FLAGS) \
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g


CXXFLAGS=-g
CCFLAGS=-g



ScriptedReadout_LDADD =	@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
			@top_srcdir@/contrib/scriptedReadout/libScriptedRdo.la	\
			@top_srcdir@/Readout/libReadout.la	\
			@top_srcdir@/DeviceSupport/libDeviceSupport.la \
			@top_srcdir@/VMEApi/libVmeAPI.la $(VMEBUILDLIBS) \
			@top_srcdir@/TCL/libtclPlus.la 	\
			@top_srcdir@/Exception/libException.la \
			$(TCL_LDFLAGS) \
			$(SPECTRODAQ_LDFLAGS) $(VMELIBRARY) -lpthread


EXTRA_DIST=Makefile.skel scriptedreadout_Makefile.include


#
#   Now the extra installation stuff:
#
#   We make:   $(prefix)/contrib/scriptedReadout
#                           ..                  /include
#                          ..                   /skel
#                            ..                 /src
#
#  All the .cpp's in this directory go to .../src
#  All the .h's in this directory go to   .../include
#  ReadoutMain.cpp, skeleton.{cpp,h} goto .../skel
#  Makefile gets generated from Makefile.skel
#  and stuff we know from configure and put
#  in                                     .../skel
#
install-exec-local:
	$(mkinstalldirs)     $(prefix)/contrib
	$(mkinstalldirs)     $(prefix)/contrib/scriptedReadout
	$(mkinstalldirs)     $(prefix)/contrib/scriptedReadout/include
	$(mkinstalldirs)     $(prefix)/contrib/scriptedReadout/skel
	$(mkinstalldirs)     $(prefix)/contrib/scriptedReadout/src
	$(mkinstalldirs)     $(prefix)/etc
	$(INSTALL_DATA)   *.cpp $(prefix)/contrib/scriptedReadout/src
	$(INSTALL_DATA)   *.h   $(prefix)/contrib/scriptedReadout/include
	$(INSTALL_DATA)  skeleton.cpp  ReadoutMain.cpp \
				$(prefix)/contrib/scriptedReadout/skel
	echo INSTDIR=$(prefix)                                 > Makefile.user
	cat  Makefile.skel                                    >> Makefile.user
	$(INSTALL_DATA) Makefile.user \
		$(prefix)/contrib/scriptedReadout/skel/Makefile
	$(INSTALL_DATA) scriptedreadout_Makefile.include \
	        $(prefix)/etc
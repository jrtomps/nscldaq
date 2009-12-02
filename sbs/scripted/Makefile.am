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
	CSIS3300Module.cpp   \
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
	CEnumParameter.cpp	\
	CV1x90Creator.cpp CV1x90Module.cpp



libScriptedRdo_la_LDFLAGS = -version-info $(SOVERSION)

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
	CSIS3300Module.h  \
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
	CEnumParameter.h	\
	CV1x90Creator.h CV1x90Module.h




INCLUDES=  $(VMEBUILDEADERS) \
	-I@top_srcdir@/base/exception \
	-I@top_srcdir@/base/tclplus \
	-I@top_srcdir@/sbs/nsclapi 	\
	-I@top_srcdir@/sbs/vmemodules\
	-I@top_srcdir@/sbs/readout   \
	-DVME $(TCL_FLAGS) \
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g


CXXFLAGS=-g
CCFLAGS=-g



ScriptedReadout_LDADD = @top_srcdir@/sbs/scripted/libscripted.la   \
	-L@top_srcdir@/sbs/readout/libReadout.la		\
	-L@top_srcdir@/sbs/vmemodules/libSBSDeviceSupport.la	\
	-L@top_srcdir@/sbs/nsclapi/libSBSVmeAPI.la		\
	-L@top_srcdir@/base/tclplus/libtclPlus.la		\
	-L@top_srcdir@/base/exception/libException.la $(TCL_LDFLAGS) \
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
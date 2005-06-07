bin_PROGRAMS = hpScriptedReadout
lib_LTLIBRARIES=libhpScriptedRdo.la



hpScriptedReadout_SOURCES = ReadoutMain.cpp  skeleton.cpp
hpScriptedReadout_DEPENDENCIES = libhpScriptedRdo.la


libhpScriptedRdo_la_SOURCES = CBoolConfigParam.cpp \
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
	CPacketCreator.cpp


libhpScriptedRdo_la_LDFLAGS = -version-info $(SOVERSION):0 

hpScriptedReadout_LDFLAGS = -Wl,"-rpath=$(prefix)/lib"


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
	CPacketCreator.h        

 

INCLUDES=  $(VMEBUILDEADERS) \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL \
	-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/DeviceSupport\
	-I@top_srcdir@/HPReadout \
	-DVME $(TCL_FLAGS) \
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g


CXXFLAGS=-g
CCFLAGS=-g



hpScriptedReadout_LDADD = -L@top_srcdir@/contrib/hpscriptedReadout -lhpScriptedRdo  \
	-L@top_srcdir@/HPReadout -lHPReadout \
	-L@top_srcdir@/DeviceSupport -lDeviceSupport \
	-L@top_srcdir@/VMEApi -lVmeAPI \
	-L@top_srcdir@/TCL -lTclPlus	\
	-L@top_srcdir@/Exception -lException $(TCL_LDFLAGS) \
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
	#
	#  Make my directory hierarchy.
	#
	$(mkinstalldirs)     $(prefix)/contrib
	$(mkinstalldirs)     $(prefix)/contrib/hpscriptedReadout
	$(mkinstalldirs)     $(prefix)/contrib/hpscriptedReadout/include
	$(mkinstalldirs)     $(prefix)/contrib/hpscriptedReadout/skel
	$(mkinstalldirs)     $(prefix)/contrib/hpscriptedReadout/src
	$(mkinstalldirs)     $(prefix)/etc
	#
	#   put the raw files where they belong:
	#
	$(INSTALL_DATA)   *.cpp $(prefix)/contrib/hpscriptedReadout/src
	$(INSTALL_DATA)   *.h   $(prefix)/contrib/hpscriptedReadout/include
	$(INSTALL_DATA)  skeleton.cpp  ReadoutMain.cpp \
				$(prefix)/contrib/hpscriptedReadout/skel
	#
	#   Now build Makefile from Makefile.skel along with other stuff
	#   that autoconf has figured out for us:
	#
	echo INSTDIR=$(prefix)                                 > Makefile.user
	cat  Makefile.skel                                    >> Makefile.user
	$(INSTALL_DATA) Makefile.user \
		$(prefix)/contrib/hpscriptedReadout/skel/Makefile
	$(INSTALL_DATA) hpscriptedreadout_Makefile.include \
	        $(prefix)/etc
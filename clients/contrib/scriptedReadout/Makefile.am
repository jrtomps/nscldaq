bin_PROGRAMS = ScriptedReadout
ScriptedReadout_SOURCES = CBoolConfigParam.cpp \
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
	ReadoutMain.cpp \
	skeleton.cpp


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
	CTriggerCommand.h 
 

INCLUDES=  $(VMEBUILDEADERS) \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL \
	-I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/DeviceSupport\
	-I@top_srcdir@/Readout \
	$(TCL_FLAGS) \
	$(SPECTRODAQ_FLAGS) -DINSTDIR=\"$(prefix)\" $(CAMAC) -g


CXXFLAGS=-g
CCFLAGS=-g


ScriptedReadout_LDADD = -L@top_srcdir@/Readout -lReadout \
	$(SPECTRODAQ_LDFLAGS)    \
	$(VMELIBRARY)            \
	-L@top_srcdir@/TCL -lTclPlus	\
	-L@top_srcdir@/Exception -lException \
	$(TCL_LDFLAGS) 

lib_LTLIBRARIES = libDeviceSupport.la
libDeviceSupport_la_SOURCES = VmeModule.cpp MmapError.cpp \
	CModule32.cpp CSIS3820.cpp sis3300.cpp CAENcard.cpp CCAENChain.cpp \
	CaenIO.cpp Nimout.cpp CAENcard_767.cpp v890.cpp CCAENV830.cpp \
	camacmap.cpp \
	CBD8210.cpp CCAMACScalerLRS2551.cpp CCAMACScalerLRS4434.cpp \
	CCAMACStatusModule.cpp CTrigger.cpp \
	CCAMACTrigger.cpp CCamac.cpp CCamacModule.cpp \
	CCamacNimout.cpp CCrateController.cpp CScaler.cpp CStatusModule.cpp \
	 CVMEScalerCAENV830.cpp CVMEScalerLRS1151.cpp \
	CVMEStatusModule.cpp CVMETrigger.cpp 

include_HEADERS = VmeModule.h CVME.h CVMEptr.h MmapError.h CSIS3820.h \
	sis3300.h CModule32.h CAENcard.h CCAENChain.h CaenIO.h Nimout.h \
	CAENcard_767.h v890.h CCAENV830.h CCAENV820Registers.h \
	camacmap.h cesmacros.h wienermacros.h macros.h cescamac.h \
	wienercamac.h camac.h \
	CBD8210.h CCAMACScalerLRS2551.h CCAMACScalerLRS4434.h \
	CCAMACStatusModule.h CTrigger.h \
	CCAMACTrigger.h CCamac.h CCamacModule.h \
	CCamacNimout.h CCrateController.h CScaler.h CStatusModule.h \
	CTrigger.h CVMEScalerCAENV830.h CVMEScalerLRS1151.h \
	CVMEStatusModule.h CVMETrigger.h CBD8210.h

libDeviceSupport_la_LDFLAGS = -version-info 1:0:0 -L@top_srcdir@/VMEApi \
	-lVmeAPI $(SPECTRODAQ_LDFLAGS) -g

INCLUDES = $(VMEBUILDHEADERS) \
	-I@top_srcdir@/Headers \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -DINSTDIR=\"$(prefix)\" $(CAMAC) -g  \
	$(SPECTRODAQ_FLAGS) 


CXXFLAGS=-g
CCFLAGS=-g
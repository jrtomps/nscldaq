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
	CVMEStatusModule.cpp CVMETrigger.cpp CCAENV1x90.cpp  \
	CCAENV977.cpp CSIS3600.cpp

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
	CVMEStatusModule.h CVMETrigger.h CBD8210.h \
	CCAENV1x90.h CCAENV1x90Data.h CCAENV1x90Opcodes.h \
	 CCAENV1x90Registers.h \
	Asserts.h \
	CCAENV977.h CSIS3600.h


libDeviceSupport_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-L@top_srcdir@/VMEApi \
	-lVmeAPI $(SPECTRODAQ_LDFLAGS) -g -Wl,"-rpath-link=$(libdir)"

INCLUDES = $(VMEBUILDHEADERS) \
	-I@top_srcdir@/Headers \
	-I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -DINSTDIR=\"$(prefix)\" $(CAMAC) -g  \
	$(SPECTRODAQ_FLAGS) 


CXXFLAGS=-g
CCFLAGS=-g


EXTRA_DIST=Doxyfile		# to build the docs

#  Documentation:

install-data-local:
	$(DOXYGEN)
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/DeviceSupport
	if [ -d docs/html ]; then  \
		(cd docs/html;     \
		for f in $$(ls);   \
		do   		   \
			$(INSTALL_DATA) $$f $(prefix)/Docs/DeviceSupport; \
		done );  \
	fi


#  Unit tests

if CPPUNIT

noinst_PROGRAMS = caenv1x90test sis3600test sis3820test

caenv1x90test_SOURCES = 1x90AcqModeTests.cpp 1x90AdjustTests.cpp \
	1x90ConstructTests.cpp 1x90DataTests.cpp 1x90MatchTiming.cpp \
	1x90MiscTests.cpp 1x90ReadTests.cpp 1x90ReadoutModes.cpp \
	1x90RegisterTests.cpp 1x90ResEdgeTests.cpp 1x90TerminationTests.cpp \
	1x90TriggerTimeTagTests.cpp 1x90geotests.cpp 1x90TestRunner.cpp 

caenv1x90test_DEPENDENCIES = CCAENV1x90.cpp VmeModule.cpp MmapError.cpp

caenv1x90test_LDADD = -L@top_srcdir@/Exception -lException \
	-L@top_srcdir@/VMEApi -lVmeAPI  $(VMEBUILDLIBS) \
	 $(SPECTRODAQ_LDFLAGS) $(CPPUNIT_LDFLAGS)

sis3600test_SOURCES = sis3600TestRunner.cpp sis3600tests.cpp 

sis3600test_DEPENDENCIES = CSIS3600.cpp \
	VmeModule.cpp MmapError.cpp CModule32.cpp

sis3600test_LDADD = -L@top_srcdir@/Exception -lException \
	-L@top_srcdir@/VMEApi -lVmeAPI $(VMEBUILDLIBS) \
	$(SPECTRODAQ_LDFLAGS) $(CPPUNIT_LDFLAGS)


sis3820test_SOURCES = sis3800TestRunner.cpp sis3820tests.cpp 

sis3820test_DEPENDENCIES = CSIS3820.cpp  VmeModule.cpp MmapError.cpp CModule32.cpp

sis3820test_LDADD = -L@top_srcdir@/Exception -lException \
	-L@top_srcdir@/VMEApi -lVmeAPI $(VMEBUILDLIBS) \
	$(SPECTRODAQ_LDFLAGS) $(CPPUNIT_LDFLAGS)

#
#  Tests will not get automatically run since the hardware requires
# preparation.


endif
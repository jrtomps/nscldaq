lib_LTLIBRARIES = libVme.la

libVme_la_SOURCES = parseUtilities.cpp
include_HEADERS   = parseUtilities.h

libVme_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"

TESTS = TestRunner.cpp testFirstWord.cpp testStripLeadingBlanks.cpp \
	testStripComment.cpp

EXTRA_DIST=$(TESTS)

ivp:
	@echo "+------------------------------------------+"
	@echo "| Building the IVP for the VME library     |"
	@echo "+------------------------------------------+"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o ivptests $(TESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-Wl,"-rpath=@prefix@/lib"
	@echo Runing ivp for libVme:
	./ivptests
	@echo done
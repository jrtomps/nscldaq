lib_LTLIBRARIES = libVme.la

libVme_la_SOURCES = parseUtilities.cpp CVMEInterface.cpp nullVMEInterface.cpp \
		    CVMEInterfaceCreator.cpp

include_HEADERS   = parseUtilities.h   CVMEInterface.h   nullVMEInterface.h \
		    CVMEInterfaceCreator.h

libVme_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"


# Tests for string parse helpers.

PARSETESTS = TestRunner.cpp testFirstWord.cpp testStripLeadingBlanks.cpp \
	testStripComment.cpp testStripTrailing.cpp testgetLine.cpp

VMEINTERFACETESTS = TestRunner.cpp testNullInterface.cpp 


TESTS = $(PARSETESTS) $(VMEINTERFACETESTS)

EXTRA_DIST=$(TESTS)

ivp:
	@echo "+------------------------------------------+"
	@echo "| Building the IVP for the VME library     |"
	@echo "+------------------------------------------+"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o parsetests $(PARSETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o vmeinterfacetests \
		$(VMEINTERFACETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-Wl,"-rpath=@prefix@/lib"
	@echo "-------------------------------------------"
	@echo Runing parsetests for libVme:
	./parsetests
	@echo Runing vmeinterfacetests
	./vmeinterfacetests
	@echo done
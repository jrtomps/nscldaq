lib_LTLIBRARIES = libVme.la

libVme_la_SOURCES = parseUtilities.cpp CVMEInterface.cpp nullVMEInterface.cpp \
		    CVMEInterfaceCreator.cpp nullVMEInterfaceCreator.cpp      \
		    CVMEInterfaceFactory.cpp CVMESubsystem.cpp CVMELock.cpp   \
		    CVMEAddressRange.cpp

include_HEADERS   = parseUtilities.h   CVMEInterface.h   nullVMEInterface.h \
		    CVMEInterfaceCreator.h nullVMEInterfaceCreator.h        \
		    CVMEInterfaceFactory.h CVMESubsystem.h CVMELock.h       \
		    CVMEAddressRange.h

AM_CXXFLAGS=-I. -I@top_srcdir@/exceptions
libVme_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"


# Tests for string parse helpers.

PARSETESTS = TestRunner.cpp testFirstWord.cpp testStripLeadingBlanks.cpp \
	testStripComment.cpp testStripTrailing.cpp testgetLine.cpp

VMEINTERFACETESTS = TestRunner.cpp testNullInterface.cpp testCreator.cpp \
	testFactory.cpp testVMESubsystem.cpp testLock.cpp 

SBSINTERFACETESTS = TestRunner.cpp testVMEAddressRange.cpp


TESTS = $(PARSETESTS) $(VMEINTERFACETESTS) Asserts.h

EXTRA_DIST=$(TESTS)

ivp:
	@echo "+------------------------------------------+"
	@echo "| Building the IVP for the VME library     |"
	@echo "+------------------------------------------+"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o parsetests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE $(PARSETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o vmeinterfacetests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE $(VMEINTERFACETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o sbsinterfacetests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE $(SBSINTERFACETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-lbtp -lpthread \
		-Wl,"-rpath=@prefix@/lib"
	@echo "-------------------------------------------"
	@echo Runing parsetests for libVme:
	./parsetests
	@echo Runing vmeinterfacetests
	./vmeinterfacetests
	@echo Running sbs inteface tests.
	./sbsinterfacetests
	@echo done
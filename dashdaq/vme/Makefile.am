lib_LTLIBRARIES = libVme.la

libVme_la_SOURCES = parseUtilities.cpp CVMEInterface.cpp nullVMEInterface.cpp \
		    CVMEInterfaceCreator.cpp nullVMEInterfaceCreator.cpp      \
		    CVMEInterfaceFactory.cpp CVMESubsystem.cpp CVMELock.cpp   \
		    CVMEAddressRange.cpp CSBSVmeAddressRange.cpp              \
		    CSBSVmeException.cpp CSBSVMEInterface.cpp                 \
		    CSBSVMEInterfaceCreator.cpp CVmeDMATransfer.cpp           \
		    CSBSVmeDMATransfer.cpp CVMEPio.cpp CSBSPio.cpp            \
		    CSimulatedVMEList.cpp CBlockReadElement.cpp               \
		    CBlockWriteElement.cpp  CCountFieldElement.cpp		\
		    CCountFieldRead.cpp CHitRegisterRead.cpp CConditionalRead.cpp

include_HEADERS   = parseUtilities.h   CVMEInterface.h   nullVMEInterface.h \
		    CVMEInterfaceCreator.h nullVMEInterfaceCreator.h        \
		    CVMEInterfaceFactory.h CVMESubsystem.h CVMELock.h       \
		    CVMEAddressRange.h CSBSVmeAddressRange.h                \
		    CSBSVmeException.h CSBSVMEInterface.h CSBSVMEInterfaceCreator.h \
		    CVmeDMATransfer.h  CSBSVmeDMATransfer.h CVMEPio.h CSBSPio.h \
		    CVMEList.h CSimulatedVMEList.h CSimulatorElement.h \
		    CBlockReadElement.h CBlockWriteElement.h CCountFieldElement.h \
		    CCountFieldRead.h CHitRegisterRead.h CConditionalRead.h

AM_CXXFLAGS=-I. -I@top_srcdir@/exceptions     \
	-I@top_srcdir@/sbsbit3/include

libVme_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"


# Tests for string parse helpers.

PARSETESTS =  testFirstWord.cpp testStripLeadingBlanks.cpp \
	testStripComment.cpp testStripTrailing.cpp testgetLine.cpp

VMEINTERFACETESTS =  testNullInterface.cpp testCreator.cpp \
	testFactory.cpp testVMESubsystem.cpp testLock.cpp \
	testVmeDMATransfer.cpp

SBSINTERFACETESTS =  testVMEAddressRange.cpp \
		testSBSRange.cpp testSBSInterface.cpp \
		testSBSCreator.cpp testSBSDMA.cpp     \
		testSBSPio.cpp

SIMULATEDLISTTESTS = testSimlistAccess.cpp testBlockReadElement.cpp  \
		testBlockWriteElement.cpp testCountFieldElement.cpp \
		testCountFieldRead.cpp testHitRead.cpp testCondRead.cpp


TESTS = $(PARSETESTS) $(VMEINTERFACETESTS) $(SBSINTERFACETESTS) \
	$(SIMULATEDLISTTESTS) TestRunner.cpp Asserts.h

EXTRA_DIST=$(TESTS)

CPPUNIT_CXXSW=-g

ivp:
	@echo "+------------------------------------------+"
	@echo "| Building the IVP for the VME library     |"
	@echo "+------------------------------------------+"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o parsetests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE TestRunner.cpp $(PARSETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-lbtp -lpthread \
		-Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o vmeinterfacetests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE TestRunner.cpp $(VMEINTERFACETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-lbtp -lpthread \
		-Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o sbsinterfacetests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE  TestRunner.cpp \
		$(SBSINTERFACETESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lVme -lException \
		-lbtp -lpthread \
		-Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o simulatedlisttests \
		$(AM_CXXFLAGS)					\
		-DUNIT_TEST_INCLUDE  TestRunner.cpp \
		$(SIMULATEDLISTTESTS) \
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
	@echo running simulated list tests.
	./simulatedlisttests
	@echo done
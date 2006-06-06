lib_LTLIBRARIES=libCamac.la

libCamac_la_SOURCES	=	CCAMACInterface.cpp \
				CCAMACInterfaceCreator.cpp

include_HEADERS		=	CCAMACInterface.h	\
				CCAMACInterfaceCreator.h



TESTHARNESS		= CFakeInterface.cpp \
			  CFakeInterface.h   \
			  TestFake.cpp \
			  TestRunner.cpp

TESTS			= TESTHARNESS


EXTRA_DIST=$(TESTS)

CPPUNIT_CXXSW=-g

ivp:
	@echo "+-------------------------------------------+"
	@echo "|   building the IVP for the CAMAC library  |"
	@echo "+-------------------------------------------+"
	@echo
	$(CXX) $(CPPUNIT_CXXSW) -c $(AM_CXXFLAGS) -I@prefix@/include \
		-DUNIT_TEST_INCLUDE CFakeInterface.cpp
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o harnessTests \
		-DUNIT_TEST_INCLUDE TestRunner.cpp TestFake.cpp CFakeInterface.o \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lCamac -lVme -lException \
		-lbtp -lpthread -Wl,"-rpath=@prefix@/lib"
	@echo "+-------------------------------------------"
	@echo
	./harnessTests
	@echo  Done
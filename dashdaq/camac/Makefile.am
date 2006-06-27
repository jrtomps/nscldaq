lib_LTLIBRARIES=libCamac.la

libCamac_la_SOURCES	=	CCAMACInterface.cpp 		\
				CCAMACInterfaceFactory.cpp	\
				CCAMACSubsystem.cpp 		\
				CCESCBD8210.cpp			\
				CCAMACCrate.cpp			\
				CBiRA1302CES8210.cpp		\
				CCES8210Creator.cpp


include_HEADERS		=	CCAMACInterface.h		\
				CCAMACInterfaceCreator.h 	\
				CCAMACInterfaceFactory.h	\
				CCAMACSubsystem.h		\
				CCESCBD8210.h			\
				CCAMACCrate.h			\
				CBiRA1302CES8210.h		\
				CCES8210Creator.h



AM_CXXFLAGS		=	-I$(top_builddir)/exceptions \
				-I$(top_builddir)/vme

libCamac_la_LIBADD	= 	$(top_builddir)/vme/libVme.la \
				$(top_builddir)/exceptions/libException.la


TESTHARNESS		= CFakeInterface.cpp \
			  CFakeInterface.h   \
			  CFakeInterfaceCreator.h \
			  CFakeInterfaceCreator.cpp \
			  TestFake.cpp \
			  TestRunner.cpp \
			  Asserts.h

FACTORYTESTS		= FactoryTest.cpp

CAMACTESTS              = subsystemTests.cpp ces8210Tests.cpp

TESTS			= $(TESTHARNESS) $(FACTORYTESTS) $(CAMACTESTS)


EXTRA_DIST=$(TESTS)

CPPUNIT_CXXSW=-g

ivp:
	@echo "+-------------------------------------------+"
	@echo "|   building the IVP for the CAMAC library  |"
	@echo "+-------------------------------------------+"
	@echo
	$(CXX) $(CPPUNIT_CXXSW) -c $(AM_CXXFLAGS) -I@prefix@/include \
		-DUNIT_TEST_INCLUDE CFakeInterface.cpp CFakeInterfaceCreator.cpp 
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o harnessTests \
		-DUNIT_TEST_INCLUDE TestRunner.cpp TestFake.cpp \
		CFakeInterface.o \
		CFakeInterfaceCreator.o \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lCamac  \
		-lbtp -lpthread -Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o factoryTests \
		-DUNIT_TEST_INCLUDE TestRunner.cpp FactoryTest.cpp \
		CFakeInterface.o \
		CFakeInterfaceCreator.o \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lCamac \
		-lbtp -lpthread -Wl,"-rpath=@prefix@/lib"
	$(CXX) -I@prefix@/include $(CPPUNIT_CXXSW) -o camacTests \
		-DUNIT_TEST_INCLUDE TestRunner.cpp $(CAMACTESTS) \
		CFakeInterface.o \
		CFakeInterfaceCreator.o \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lCamac \
		-lbtp -lpthread -Wl,"-rpath=@prefix@/lib"
	@echo "+-------------------------------------------"
	@echo
	./harnessTests
	./factoryTests
	./camacTests
	@echo  Done
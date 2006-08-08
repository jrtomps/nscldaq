lib_LTLIBRARIES	= libDeviceSupport.la

libDeviceSupport_la_SOURCES	= CCAMACModule.cpp	\
				CVMEModule.cpp		\
				CCAMACReadableObject.cpp \
				CVMEReadableObject.cpp	\
				CAEN32.cpp

include_HEADERS     = CReadableObject.h	\
			CCAMACModule.h	\
			CVMEModule.h 	\
			CCAMACReadableObject.h	\
			CVMEReadableObject.h	\
			CAEN32.h


AM_CXXFLAGS	= -I@top_builddir@/vme    \
		-I@top_builddir@/camac	\
		-I@top_builddir@/exceptions

libDeviceSupport_la_LIBADD = @top_builddir@/camac/libCamac.la 	\
			     @top_builddir@/vme/libVme.la	\
			     @top_builddir@/exceptions/libException.la

TESTS = caen32tests.cpp

EXTRA_DIST=$(TESTS) TestRunner.cpp Asserts.h

CPPUNIT_CXXSW= -g -DUNIT_TEST_INCLUDE -I@prefix@/include $(AM_CXXFLAGS) \
	-L@prefix@/lib

ivp:
	@echo "+-------------------------------------------------+"
	@echo "| Building the IVP for the device support library |"
	@echo "+-------------------------------------------------+"
	$(CXX) $(CPPUNIT_CXXSW) -o tests TestRunner.cpp $(TESTS) -lDeviceSupport \
	-Wl,"-rpath=@prefix@/lib" -lcppunit -lbtp -lpthread
	@echo Running device support tests:
	./tests
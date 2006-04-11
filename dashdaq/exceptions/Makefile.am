lib_LTLIBRARIES = libException.la
libException_la_SOURCES = ErrnoException.cpp Exception.cpp \
			  RangeError.cpp StreamIOError.cpp \
			CInvalidInterfaceType.cpp
include_HEADERS = ErrnoException.h Exception.h  StreamIOError.h \
	RangeError.h CInvalidInterfaceType.h

libException_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"


man_MANS=streamioerror.3daq rangeerror.3daq exception.3daq \
	errnoexception.3daq

TESTS= testErrnoException.cpp  \
	testException.cpp  \
	testRangeError.cpp  \
	testBadInterface.cpp \
	testStreamIOError.cpp  TestRunner.cpp

EXTRA_DIST=streamioerror.3daq rangeerror.3daq exception.3daq \
	errnoexception.3daq $(TESTS) Asserts.h


ivp: 
	@echo	"+-------------------------------------+"
	@echo   "| Building the ivp for exceptions     |"
	@echo   "+-------------------------------------+"
	$(CXX)   -I@prefix@/include $(CPPUNIT_CXXSW) -o ivptests $(TESTS) \
		$(CPPUNIT_LDSW) -L@prefix@/lib -lException -Wl,"-rpath=@prefix@/lib"
	@echo  Running the ivp for exceptions:
	./ivptests
	@echo done

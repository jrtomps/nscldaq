lib_LTLIBRARIES = libException.la
libException_la_SOURCES = ErrnoException.cpp Exception.cpp \
			  RangeError.cpp StreamIOError.cpp 
include_HEADERS = ErrnoException.h Exception.h  StreamIOError.h \
	RangeError.h
libException_la_LDFLAGS = -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"

INCLUDES = -I@top_srcdir@/Headers

install-data-local:
	rm -f $(prefix)/include/CErrnoException.h
	$(LN_S) $(prefix)/include/ErrnoException.h \
	        $(prefix)/include/CErrnoException.h

man_MANS=streamioerror.3daq rangeerror.3daq exception.3daq \
	errnoexception.3daq

EXTRA_DIST=streamioerror.3daq rangeerror.3daq exception.3daq \
	errnoexception.3daq


if CPPUNIT
noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp testException.cpp testErrnoException.cpp \
	testStreamIOError.cpp testRangeError.cpp

unittests_LDADD   = $(CPPUNIT_LDFLAGS) -lException

noinst_HEADERS = Asserts.h


TESTS=./unittests
endif
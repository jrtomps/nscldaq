lib_LTLIBRARIES = libException.la
libException_la_SOURCES = ErrnoException.cpp Exception.cpp \
			  RangeError.cpp StreamIOError.cpp \
			StateException.cpp URIFormatException.cpp \
			MonitorException.cpp CInvalidArgumentException.cpp

include_HEADERS = ErrnoException.h Exception.h  StreamIOError.h \
	RangeError.h StateException.h URIFormatException.h \
	MonitorException.h CInvalidArgumentException.h

libException_la_LDFLAGS = -version-info $(SOVERSION) \
	-Wl,"-rpath-link=$(libdir)"

INCLUDES = -I@top_srcdir@/base/headers

install-data-local:
	rm -f $(prefix)/include/CErrnoException.h
	$(LN_S) $(prefix)/include/ErrnoException.h \
	        $(prefix)/include/CErrnoException.h

EXTRA_DIST=exceptions.xml



noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp testException.cpp testErrnoException.cpp \
	testStreamIOError.cpp testRangeError.cpp

unittests_LDADD   = $(CPPUNIT_LDFLAGS) -lException

noinst_HEADERS = Asserts.h


TESTS=./unittests

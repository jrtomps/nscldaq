lib_LTLIBRARIES = libLicense.la
libLicense_la_SOURCES = CopyrightNotice.cpp

INCLUDES =  -I@top_srcdir@/base/headers



include_HEADERS = CopyrightNotice.h
libLicense_la_LDFLAGS =  -version-info $(SOVERSION) \
	-Wl,"-rpath-link=$(libdir)"

TESTS = ./test

noinst_PROGRAMS = test

test_SOURCES       = test.cpp

test_LDADD = CopyrightNotice.o


EXTRA_DIST = copyrightnotice.xml

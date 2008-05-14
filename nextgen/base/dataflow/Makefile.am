lib_LTLIBRARIES = libDataFlow.la
libDataFlow_la_SOURCES = CRingBuffer.cpp
include_HEADERS        = CRingBuffer.h
noinst_HEADERS         = ringbufint.h Asserts.h

INCLUDES = -I@top_srcdir@/base/exception \
	   -I@top_srcdir@/base/headers 

libDataFlow_la_LDFLAGS =  -version-info $(SOVERSION) \
	                  -Wl,"-rpath-link=$(libdir)" -lrt
libDataFlow_la_LIBADD  = @top_srcdir@/base/exception/libException.la



noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp StaticTests.cpp
unittests_LDADD   = -L@prefix@/lib $(CPPUNIT_LDFLAGS) -lDataFlow -lException  
unittests_LDFLAGS = -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests
lib_LTLIBRARIES = libdaqshm.la

libdaqshm_la_SOURCES = daqshm.cpp os.cpp io.cpp
include_HEADERS      = daqshm.h os.h io.h

noinst_HEADERS	     = Asserts.h

libdaqshm_la_LIBADD = @THREADLD_FLAGS@ -lcrypt
INCLUDES            = @THREADCXX_FLAGS@


EXTRA_DIST=shm.xml

#----------------------------------------
#
# Tests:

noinst_PROGRAMS    = unittests
unittests_SOURCES = TestRunner.cpp createTests.cpp removeTests.cpp attachTests.cpp \
	detachTests.cpp

unittests_LDADD   = libdaqshm.la @CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

TESTS=./unittests
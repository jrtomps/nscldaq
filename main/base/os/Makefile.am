lib_LTLIBRARIES = libdaqshm.la

<<<<<<< HEAD
libdaqshm_la_SOURCES = daqshm.cpp os.cpp io.cpp
include_HEADERS      = daqshm.h os.h io.h
=======
libdaqshm_la_SOURCES = daqshm.cpp os.cpp io.cpp CTimeout.cpp
include_HEADERS      = daqshm.h os.h io.h CTimeout.h
>>>>>>> master

noinst_HEADERS	     = Asserts.h

libdaqshm_la_LIBADD = @THREADLD_FLAGS@ -lcrypt @LIBEXCEPTION_LDFLAGS@
COMPILATION_FLAGS   = @THREADCXX_FLAGS@ @LIBTCLPLUS_CFLAGS@
libdaqshm_la_CPPFLAGS=$(COMPILATION_FLAGS)


EXTRA_DIST=shm.xml

#----------------------------------------
#
# Tests:

noinst_PROGRAMS    = unittests
unittests_SOURCES = TestRunner.cpp createTests.cpp removeTests.cpp attachTests.cpp \
<<<<<<< HEAD
	detachTests.cpp
=======
        detachTests.cpp timeoutTests.cpp
>>>>>>> master

unittests_CPPFLAGS=$(COMPILATION_FLAGS)

unittests_LDADD   = libdaqshm.la @CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

TESTS=./unittests

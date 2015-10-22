bin_PROGRAMS =dumpend

dumpend_SOURCES = CFileIterator.cpp CFileIterator.h


noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp fileitertest.cpp \
	CFileIterator.cpp CFilIterator.h

unittests_CXXFLAGS=$(CPPUNIT_CFLAGS)
unittests_LDFLAGS=$(CPPUNIT_LDFLAGS)

TESTS=unittests
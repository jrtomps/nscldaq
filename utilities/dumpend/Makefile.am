bin_PROGRAMS =dumpend

COMMON_CFLAGS = -I@top_srcdir@/daq/format @LIBTCLPLUS_CFLAGS@
COMMON_LOADFLAGS= @top_srcdir@/daq/format/libdataformat.la @top_srcdir@/base/dataflow/libDataFlow.la \
	@LIBEXCEPTION_LDFLAGS@

dumpend_SOURCES = CFileIterator.cpp CFileIterator.h main.cpp CEndRunInfo.cpp CEndRunInfo.h \
	CEndRunInfo11.cpp CEndRunInfo11.h

dumpend_CXXFLAGS=$(COMMON_CFLAGS)
dumpend_LDFLAGS=$(COMMON_LOADFLAGS)

noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp fileitertest.cpp \
	endinfo11test.cpp			\
	CFileIterator.cpp CFileIterator.h \
	CEndRunInfo.cpp CEndRunInfo.h	\
	CEndRunInfo11.cpp CEndRunInfo11.h

unittests_CXXFLAGS=$(CPPUNIT_CFLAGS) $(COMMON_CFLAGS) -I@top_srcdir@/base/os
unittests_LDFLAGS=$(CPPUNIT_LDFLAGS) @top_srcdir@/base/os/libdaqshm.la $(COMMON_LOADFLAGS) 

TESTS=unittests
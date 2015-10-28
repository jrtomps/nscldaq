bin_PROGRAMS =dumpend

COMMON_CFLAGS = -I@top_srcdir@/daq/format -I@top_srcdir@/base/os @LIBTCLPLUS_CFLAGS@
COMMON_LOADFLAGS= @top_builddir@/daq/format/libdataformat.la @top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@

dumpend_SOURCES = CFileIterator.cpp CFileIterator.h main.cpp CEndRunInfo.cpp CEndRunInfo.h \
	CEndRunInfo11.cpp CEndRunInfo11.h CEndRunInfo10.cpp CEndRunInfo10.h \
	CEndRunInfoFactory.cpp CEndRunInfoFactory.h

dumpend_CXXFLAGS=$(COMMON_CFLAGS)
dumpend_LDFLAGS=$(COMMON_LOADFLAGS)

noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp fileitertest.cpp \
	endinfo11test.cpp endrun10test.cpp	\
	CFileIterator.cpp CFileIterator.h \
	CEndRunInfo.cpp CEndRunInfo.h	\
	CEndRunInfo11.cpp CEndRunInfo11.h \
	CEndRunInfo10.cpp CEndRunInfo10.h \
	CEndRunInfoFactory.cpp CEndRunInfoFactory.h

unittests_CXXFLAGS=$(CPPUNIT_CFLAGS) $(COMMON_CFLAGS) -I@top_srcdir@/base/os
unittests_LDFLAGS=$(CPPUNIT_LDFLAGS) @top_builddir@/base/os/libdaqshm.la $(COMMON_LOADFLAGS) 

TESTS=unittests
lib_LTLIBRARIES = libStatusMessages.la

COMPILATION_FLAGS= -I@top_srcdir@/base/os
COMMON_LIBS      = @top_builddir@/base/os/libdaqshm.la  @ZMQ_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@

libStatusMessages_la_SOURCES=Constants.cpp CStatusDefinitions.h \
	CRingStatisticsMessage.cpp \
	CReadoutStatistics.cpp

libStatusMessages_la_CPPFLAGS = $(COMPILATION_FLAGS)
libStatusMessages_la_LDFLAGS  = $(COMMON_LIBS)

include_HEADERS = CStatusMessage.h

#-------------------- Tests -----


TEST_PROGS=ringstattests

noinst_PROGRAMS=$(TEST_PROGS)


ringstattests_SOURCES=TestRunner.cpp Asserts.h ringstattests.cpp
ringstattests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
ringstattests_LDFLAGS=libStatusMessages.la $(COMMON_LIBS) @CPPUNIT_LDFLAGS@

TESTS=ringstattests

lib_LTLIBRARIES = libStatusMessages.la

COMPILATION_FLAGS=
COMMON_LIBS      = @ZMQ_LDFLAGS@

libStatusMessages_la_SOURCES=CStatusMessage.cpp
libStatusMessages_la_CPPFLAGS = $(COMPILATION_FLAGS)
libStatusMessages_la_LDFLAGS  = $(COMMON_LIBS)

include_HEADERS = CStatusMessage.h

TEST_PROGS=ringstattests

noinst_PROGRAMS=$(TEST_PROGS)


ringstattests_SOURCES=TestRunner.cpp Asserts.h ringstattests.cpp
ringstattests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
ringstattests_LDFLAGS=libStatusMessages.la $(COMMON_LIBS) @CPPUNIT_LDFLAGS@

TESTS=ringstattests

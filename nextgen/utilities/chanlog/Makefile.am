#  Makefile template for chanlog : log epics channels to file or stdout.
#  This can only be built if Epics was detected.

if EPICS
bin_PROGRAMS = chanlog


chanlog_SOURCES = chanlog.cpp getinfo.cpp
noinst_HEADERS = getinfo.h
chanlog_LDADD   = $(EPICS_LDFLAGS) $(THREADLD_FLAGS)
chanlog_CXXFLAGS= $(THREADCXX_FLAGS) $(AM_CXXFLAGS)


INCLUDES        = $(EPICS_BIN) $(EPICS_INCLUDES) -I@top_srcdir@/Headers

EXTRA_DIST = chanlog.xml

endif
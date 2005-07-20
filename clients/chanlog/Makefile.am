#  Makefile template for chanlog : log epics channels to file or stdout.

bin_PROGRAMS = chanlog

chanlog_SOURCES = chanlog.cpp getinfo.cpp
include_HEADERS = getinfo.h
chanlog_LDADD   = $(EPICS_LDFLAGS)
INCLUDES        = $(EPICS_BIN) $(EPICS_INCLUDES) -I@top_srcdir@/Headers


man_MANS        =  chanlog.1
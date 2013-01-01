lib_LTLIBRARIES   = libcvt.la
libcvt_la_SOURCES = daqcvt.c
include_HEADERS   = daqcvt.h
libcvt_MANS       = libcvt.ps
libcvt_la_LDFLAGS = -version-info @SOVERSION@ -Wl,"-rpath-link=$(libdir)" \
			$(THREADLD_FLAGS)
libcvt_la_CXXFLAGS= $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

EXTRA_DIST=cvtdoc.xml
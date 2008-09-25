

lib_LTLIBRARIES=liburl.la

liburl_la_SOURCES = URL.cpp
include_HEADERS = URL.h

INCLUDES = -I@top_srcdir@/base/headers -I@top_srcdir@/base/exception

liburl_la_LDFLAGS	=	$(THREADLD_FLAGS)
liburl_la_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS) 

EXTRA_DIST = URL.xml

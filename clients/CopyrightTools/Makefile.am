lib_LTLIBRARIES = libLicense.la
libLicense_la_SOURCES = CopyrightNotice.cpp

include_HEADERS = CopyrightNotice.h
libLicense_la_LDFLAGS =  -version-info $(SOVERSION):0 \
	-Wl,"-rpath-link=$(libdir)"

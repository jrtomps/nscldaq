lib_LTLIBRARIES = libException.la
libException_la_SOURCES = ErrnoException.cpp Exception.cpp \
			  RangeError.cpp StreamIOError.cpp 
include_HEADERS = ErrnoException.h Exception.h  StreamIOError.h \
	CRangeError.h RangeError.h
libException_la_LDFLAGS = -version-info $(SOVERSION):0

INCLUDES = -I@top_srcdir@/Headers

install-data-local:
	rm -f $(prefix)/include/CErrnoException.h
	$(LN_S) $(prefix)/include/ErrnoException.h \
	        $(prefix)/include/CErrnoException.h




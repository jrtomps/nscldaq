lib_LTLIBRARIES = libException.la
libException_la_SOURCES = ErrnoException.cpp Exception.cpp \
			  RangeError.cpp StreamIOError.cpp 
include_HEADERS = ErrnoException.h Exception.h RangeError.h StreamIOError.h 
libException_la_LDFLAGS = -version-info 1:0:0

INCLUDES = -I@top_srcdir@/Headers

install-data-local:
	rm -f $(prefix)/include/CRangeError.h
	rm -f $(prefix)/include/CErrnoException.h
	$(LN_S) $(prefix)/include/RangeError.h $(prefix)/include/CRangeError.h
	$(LN_S) $(prefix)/include/ErrnoException.h \
	        $(prefix)/include/CErrnoException.h




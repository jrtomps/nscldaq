lib_LTLIBRARIES = libException.la
libException_la_SOURCES = ErrnoException.cpp Exception.cpp \
			  RangeError.cpp StreamIOError.cpp 
include_HEADERS = ErrnoException.h Exception.h RangeError.h StreamIOError.h 
libException_la_LDFLAGS = -version-info 1:0:0

INCLUDES = -I@top_srcdir@/Headers
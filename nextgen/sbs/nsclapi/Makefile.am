lib_LTLIBRARIES = libSBSVmeAPI.la
libSBSVmeAPI_la_SOURCES = SBSBIT3API.cpp Locking.cpp
include_HEADERS = CVMEInterface.h SBSBit3API.h 

# Locking.cpp


libSBSVmeAPI_la_LDFLAGS =  -version-info 1:0:0  \
	-Wl"-rpath=$(libdir)"

libSBSVmeAPI_la_LIBADD = @top_srcdir@/base/exception/libException.la	\
			@top_srcdir@/sbs/driver/src/libbtp.la


INCLUDES = -I@top_srcdir@/base/headers -I@top_srcdir@/base/exception	\
	-I@top_srcdir@/sbs/driver/include

LDFLAGS = -lpthread




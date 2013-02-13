lib_LTLIBRARIES = libTcp.la

libTcp_la_SOURCES = CSocket.cpp \
	CTCPBadSocketState.cpp \
	CTCPConnectionFailed.cpp \
	CTCPConnectionLost.cpp \
	CTCPNoSuchHost.cpp \
	CTCPNoSuchService.cpp

include_HEADERS = CSocket.h \
	CTCPBadSocketState.h \
	CTCPConnectionFailed.h \
	CTCPConnectionLost.h \
	CTCPNoSuchHost.h \
	CTCPNoSuchService.h


INCLUDES = -I@top_srcdir@/base/exception \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os

libTcp_la_LIBADD = @top_builddir@/base/exception/libException.la \
	@top_builddir@/base/os/libdaqshm.la

lib_LTLIBRARIES		= libCCUSBThreadComm.la
libCCUSBThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp
noinst_HEADERS		= CGaurdedObject.h CBufferQueue.h

INCLUDES		= -I@top_srcdir@/base/exception -I@top_srcdir@/base/headers
libCCUSBThreadComm_la_LIBADD = @top_builddir@/base/exception/libException.la


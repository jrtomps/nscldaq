lib_LTLIBRARIES		= libThreadComm.la
libThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp
include_HEADERS		= CGaurdedObject.h CBufferQueue.h

INCLUDES		= -I$(top_builddir)/base/exception -I$(top_builddir)/base/headers
libThreadComm_la_LIBADD =  $(top_builddir)/base/exception/libException.la


install-data:
	$(INSTALL_DATA) @srcdir@CBufferQueue.cpp @prefix@/include
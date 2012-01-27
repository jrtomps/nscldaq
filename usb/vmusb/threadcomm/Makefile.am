lib_LTLIBRARIES		= libThreadComm.la
libThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp
noinst_HEADERS		= CGaurdedObject.h CBufferQueue.h

libThreadComm_la_CXXFLAGS = @THREADCXX_FLAGS@

INCLUDES		= -I$(top_builddir)/base/exception -I$(top_builddir)/base/headers
libThreadComm_la_LIBADD =  $(top_builddir)/base/exception/libException.la @THREADLD_FLAGS@


install-data:
	$(INSTALL_DATA) @srcdir@CBufferQueue.cpp @prefix@/include
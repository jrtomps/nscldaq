lib_LTLIBRARIES		= libVMUSBThreadComm.la
libVMUSBThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp
noinst_HEADERS		= CGaurdedObject.h CBufferQueue.h

libVMUSBThreadComm_la_CXXFLAGS = @THREADCXX_FLAGS@

INCLUDES		= -I$(top_builddir)/base/exception -I$(top_builddir)/base/headers
libVMUSBThreadComm_la_LIBADD =  $(top_builddir)/base/exception/libException.la @THREADLD_FLAGS@


install-data:
	$(INSTALL_DATA) @srcdir@CBufferQueue.cpp @prefix@/include
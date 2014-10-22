##
#  This makefile is obsolete and is no longer used.  At some point this directory
#  and its contents should be removed from the source tree.

lib_LTLIBRARIES		= libCCUSBThreadComm.la
libCCUSBThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp
noinst_HEADERS		= CGaurdedObject.h CBufferQueue.h

INCLUDES		= @LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/headers \
	@THREADCXX_FLAGS@
libCCUSBThreadComm_la_LIBADD = @LIBTCLPLUS_LDFLAGS@ @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
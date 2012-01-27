lib_LTLIBRARIES		= libThreadComm.la
libThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp
include_HEADERS		= CGaurdedObject.h CBufferQueue.h

INCLUDES		= $(SPECTRODAQ_FLAGS)
libThreadComm_la_LIBADD = $(SPECTRODAQ_LDFLAGS)


install-data:
	$(INSTALL_DATA) @srcdir@CBufferQueue.cpp @prefix@/include
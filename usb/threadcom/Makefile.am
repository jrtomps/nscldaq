lib_LTLIBRARIES		= libThreadComm.la
libThreadComm_la_SOURCES = CGaurdedObject.cpp CBufferQueue.cpp \
				CControlQueues.cpp		\
				CRunState.cpp

noinst_HEADERS		= CGaurdedObject.h CBufferQueue.h	\
			CControlQueues.h			\
			CRunState.h

INCLUDES	=	-I@top_srcdir@/base/thread

libThreadComm_la_LIBADD 	=	@top_srcdir@/base/thread/libdaqthreads.la	\
					@top_srcdir@/base/exception/libException.la


install-data:
	$(INSTALL_DATA) @srcdir@/CBufferQueue.cpp @prefix@/include
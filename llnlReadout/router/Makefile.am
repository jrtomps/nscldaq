lib_LTLIBRARIES		= 	libRouter.la
libRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

include_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

INCLUDES		= $(NSCLDAQ_FLAGS) $(SPECTRODAQ_FLAGS) \
				-I@top_srcdir@/threadcomm

libRouter_la_LIBADD	= $(NSCLDAQ_LIB) $(SPECTRODAQ_LDFLAGS)	\
			@top_srcdir@/threadcomm/libThreadComm.la
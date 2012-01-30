lib_LTLIBRARIES		= 	libRouter.la
libRouter_la_SOURCES	= 	COutputThread.cpp	\
				CRunState.cpp  		\
				DataBuffer.cpp

include_HEADERS		=	COutputThread.h		\
				CRunState.h  		\
				DataBuffer.h

INCLUDES		= $(NSCLDAQ_FLAGS)  \
				-I@top_srcdir@/threadcomm -DLAST_CHANCE

libRouter_la_LIBADD	= @top_srcdir@/threadcomm/libThreadComm.la
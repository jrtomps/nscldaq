lib_LTLIBRARIES = libdaqthreads.la
libdaqthreads_la_SOURCES = dshwrappthreads.cpp  SyncGuard.cpp  Synchronizable.cpp  \
				Thread.cpp	\
				CMutex.cpp	\
				CCondition.cpp
include_HEADERS = dshwrappthreads.h  dshwrapthreads.h  Runnable.h  SyncGuard.h  \
		Synchronizable.h  Thread.h \
		CMutex.h		\
		CCondition.h

INCLUDES = -I$(top_srcdir)/base/headers -I$(top_srcdir)/base/exception -DUSE_PTHREADS

libdaqthreads_la_LDFLAGS = -version-info $(SOVERSION) \
		-Wl,"-rpath-link=$(libdir)"


EXTRA_DIST=thread.xml
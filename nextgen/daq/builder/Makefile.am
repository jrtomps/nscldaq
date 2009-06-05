lib_LTLIBRARIES 	= libBuilder.la

libBuilder_la_SOURCES 	= CFragmentSource.cpp	\
			CSourceException.cpp

include_HEADERS	        = CFragmentSource.h	\
			CSourceException.h

INCLUDES		= -I@top_srcdir@/base/thread

libBuilder_la_LDFLAGS	= -versino-info $(SOVERSION)	\
			-Wl,"-rpath-link=$(libdir)" 	 \
			$(THREADLD_FLAGS)

libBuilder_la_LIBADD	= @top_srcdir@/base/thread/libdaqthreads.la	\
			@top_srcdir@/base/exception/libException.la

#---------- Test:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp 		\
			testSource.cpp

unittests_LDADD		= CFragmentSource.lo		\
			@top_srcdir@/base/thread/libdaqthreads.la	\
			@top_srcdir@/base/exception/libException.la	\
			$(CPPUNIT_LDFLAGS)


TESTS=./unittests
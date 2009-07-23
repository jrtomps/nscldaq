lib_LTLIBRARIES 	= libBuilder.la

libBuilder_la_SOURCES 	= CFragmentSource.cpp	
#			CSourceException.cpp	\
#			CInvalidObjectType.cpp	\
#			CNoSuchConfigurationParameter.cpp	\
#			CInvalidConfigurationValue.cpp	\
#			CInvalidObjectState.cpp	\
#			CNoSuchManager.cpp	\
#			CFragmentSourceManager.cpp

include_HEADERS	        = BuilderContants.h	\
			CFragmentSource.h 
#			CSourceException.h	\
#			CInvalidObjectType.h	\
#			CNoSuchConfiguratinoParameter.h	\
#			CInvalidConfigurationValue.h	\
#			CInvalidObjectState.h	\
#			CNoSuchManager.h	\
#			CFragmentSourceManager.h


INCLUDES		= -I@top_srcdir@/base/thread

libBuilder_la_LDFLAGS	= -versino-info $(SOVERSION)	\
			-Wl,"-rpath-link=$(libdir)" 	 \
			$(THREADLD_FLAGS)

libBuilder_la_LIBADD	= @top_srcdir@/base/thread/libdaqthreads.la	\
			@top_srcdir@/base/exception/libException.la

#---------- Test:

#noinst_PROGRAMS = unittests

#unittests_SOURCES	= TestRunner.cpp 		\
#			testSource.cpp

#unittests_LDADD		= CFragmentSource.lo		\
#			@top_srcdir@/base/thread/libdaqthreads.la	\
#			@top_srcdir@/base/exception/libException.la	\
#			$(CPPUNIT_LDFLAGS)
#
#
#TESTS=./unittests
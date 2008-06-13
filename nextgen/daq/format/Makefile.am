#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdataformat.la

libdataformat_la_SOURCES = CRingSelectionPredicate.cpp	\
			   CDesiredTypesPredicate.cpp

include_HEADERS		= CRingSelectionPredicate.h	\
			  CDesiredTypesPredicate.h

INCLUDES	= -I@top_srcdir@/base/dataflow

libdataformat_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt

libdataformat_la_LIBADD	= @top_srcdir@/base/dataflow/libDataFlow.la


#------------------- Tests:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp selecttest.cpp desiredtests.cpp

unittests_LDADD		= -L$(libdir) $(CPPUNIT_LDFLAGS) -ldataformat -lDataFlow
unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
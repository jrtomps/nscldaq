#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdataformat.la

libdataformat_la_SOURCES = CRingSelectionPredicate.cpp	\
			   CDesiredTypesPredicate.cpp	\
			   CAllButPredicate.cpp	\
			CRingItem.cpp \
			CRingStateChangeItem.cpp	\
			CRingScalerItem.cpp		\
			CRingTextItem.cpp		\
			CRingPhysicsEventCountItem.cpp


include_HEADERS	= CRingSelectionPredicate.h	\
			  CDesiredTypesPredicate.h	\
			  CAllButPredicate.h	\
			 CRingItem.h \
			CRingStateChangeItem.h	\
			CRingScalerItem.h	\
			CRingTextItem.h	\
			CRingPhysicsEventCountItem.h	\
			DataFormat.h

INCLUDES	= -I@top_srcdir@/base/dataflow	\
		  -I@top_srcdir@/base/exception	\
		-I@top_srcdir@/base/headers

libdataformat_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt

libdataformat_la_LIBADD	= @top_srcdir@/base/dataflow/libDataFlow.la \
			@top_srcdir@/base/exception/


#------------------- Tests:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp selecttest.cpp desiredtests.cpp	\
			  allbuttests.cpp ringitemtests.cpp teststate.cpp	\
			scalertests.cpp texttest.cpp physcounttests.cpp

unittests_LDADD		= -L$(libdir) $(CPPUNIT_LDFLAGS) -ldataformat -lDataFlow
unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
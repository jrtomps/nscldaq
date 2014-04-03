#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdataformat.la

libdataformat_la_SOURCES = CRingSelectionPredicate.cpp	\
			   CDesiredTypesPredicate.cpp	\
			   CAllButPredicate.cpp	\
			CRingItem.cpp \
			CRingStateChangeItem.cpp	\
			CRingScalerItem.cpp		\
			CRingTextItem.cpp		\
			CRingPhysicsEventCountItem.cpp \
			CRingFragmentItem.cpp       \
			CPhysicsEventItem.cpp       \
                        CDataFormatItem.cpp          \
                        CGlomParameters.cpp              \
                        CUnknownFragment.cpp            \
			CRingItemFactory.cpp		\
			ringitem.c			\
      RingItemComparisons.cpp \
			CDataSource.cpp CRingDataSource.cpp CFileDataSource.cpp  \
			CDataSourceFactory.cpp \
      CFileDataSink.cpp CDataSinkFactory.cpp \
      CRingDataSink.cpp


include_HEADERS	= CRingSelectionPredicate.h	\
			  CDesiredTypesPredicate.h	\
			  CAllButPredicate.h	\
			 CRingItem.h \
			CRingStateChangeItem.h	\
			CRingScalerItem.h	\
			CRingTextItem.h	\
			CRingPhysicsEventCountItem.h	\
			CRingFragmentItem.h           \
			CPhysicsEventItem.h	\
			CRingItemFactory.h	\
                        CDataFormatItem.h        \
                        CGlomParameters.h       \
                        CUnknownFragment.h      \
			DataFormat.h	\
      RingItemComparisons.h \
			CDataSource.h CRingDataSource.h CFileDataSource.h \
			CDataSourceFactory.h \
      CDataSink.h CFileDataSink.h CDataSinkFactory.h \
      CRingDataSink.h


INCLUDES	= -I@top_srcdir@/base/dataflow	\
		@LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/headers	\
		-I@top_srcdir@/base/uri		\
		-I@top_srcdir@/base/os

libdataformat_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libdataformat_la_LIBADD	= @top_builddir@/base/dataflow/libDataFlow.la \
			@top_builddir@/base/uri/liburl.la		\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@


libdataformat_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

#------------------- Tests:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp selecttest.cpp desiredtests.cpp	\
			  allbuttests.cpp ringitemtests.cpp teststate.cpp	\
			scalertests.cpp texttest.cpp physcounttests.cpp		\
			formatoutputtests.cpp dataformattests.cpp       \
                        fragmenttest.cpp glomparamtests.cpp factorytests.cpp \
                      physeventtests.cpp filedatasinktests.cpp \
                     datasinkfactorytests.cpp \
                     datasourcefactorytests.cpp

unittests_LDADD		= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/daq/format/libdataformat.la	\
			@LIBEXCEPTION_LDFLAGS@



unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests

EXTRA_DIST		=	dataformat.xml

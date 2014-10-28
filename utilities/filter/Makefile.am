#-------------- The library libdataformat

lib_LTLIBRARIES	=	libfilter.la

libfilter_la_SOURCES = CFilterMain.cpp \
                       CMediator.cpp \
                       CInfiniteMediator.cpp \
                       COneShotMediator.cpp \
                       COneShotHandler.cpp \
                       CCompositeFilter.cpp \
                       COneShotException.cpp \
                       CMediatorException.cpp \
                       @srcdir@/../StringsToIntegers.cpp \
                       @srcdir@/filterargs.c
                       
BUILT_SOURCES = @srcdir@/filterargs.c

noinst_HEADERS = @srcdir@/../StringsToIntegers.h \
                 @srcdir@/filterargs.h

include_HEADERS	=  CFilterMain.h \
                   CMediator.h \
                   CInfiniteMediator.h \
                   COneShotMediator.h \
                   COneShotHandler.h \
                   CFilter.h \
                   CCompositeFilter.h  \
                   CTransparentFilter.h \
                   CNullFilter.h \
                   COneShotException.h \
                   CMediatorException.h \
                   CFatalException.h

libfilter_la_CPPFLAGS	= -I@top_srcdir@/utilities \
    @LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
    -I@top_srcdir@/daq/format \
    -I@top_srcdir@/base/dataflow

libfilter_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libfilter_la_LIBADD	= @top_builddir@/daq/format/libdataformat.la \
			@top_builddir@/base/dataflow/libDataFlow.la \
			@top_builddir@/base/uri/liburl.la		\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@


libfilter_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)


libfilter_la_DEPENDENCIES = @srcdir@/filterargs.h

@srcdir@/filterargs.c: @srcdir@/filterargs.h

@srcdir@/filterargs.h: @srcdir@/filterargs.ggo 
	$(GENGETOPT) < @srcdir@/filterargs.ggo \
		--output-dir=@srcdir@ --file=filterargs --unamed-opts="OPTIONS"



#------------------- Tests:

noinst_PROGRAMS = unittests testapp

unittests_SOURCES	= TestRunner.cpp  \
            infinitemediatortests.cpp \
            filtermaintests.cpp  \
            compositefiltertests.cpp \
            transparentfiltertests.cpp \
            oneshothandlertests.cpp \
            oneshotmediatortests.cpp 

unittests_LDADD		= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/daq/format/libdataformat.la	\
			@top_builddir@/utilities/filter/libfilter.la	\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=  -I@top_srcdir@/utilities \
    @LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
    -I@top_srcdir@/daq/format \
    -I@top_srcdir@/base/dataflow
unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


testapp_SOURCES = TestApp.cpp
testapp_CPPFLAGS= -I@top_srcdir@/utilities \
    @LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
    -I@top_srcdir@/daq/format \
    -I@top_srcdir@/base/dataflow
testapp_LDADD = -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/daq/format/libdataformat.la	\
			@top_builddir@/utilities/filter/libfilter.la	\
			@LIBEXCEPTION_LDFLAGS@
testapp_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests

#-------------------- Filter Kit

install-data-local:
	$(mkinstalldirs) @prefix@/filterkit
	$(INSTALL_DATA) FilterIncludes \
		@srcdir@/CTemplateFilter.cpp \
    @srcdir@/SkeletonMain.cpp @prefix@/filterkit
	$(INSTALL_DATA) Makefile-template @prefix@/filterkit/Makefile

EXTRA_DIST = filterargs.ggo SkeletonMain.cpp CTemplateFilter.cpp \
             Makefile-template.in FilterIncludes.in

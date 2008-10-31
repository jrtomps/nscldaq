lib_LTLIBRARIES	=	libSBSProductionReadout.la

libSBSProductionReadout_la_SOURCES	=	options.c \
						CReadoutMain.cpp	\
						CTCLAuthorizer.cpp	\
						CAuthorizedTclServer.cpp	\
						CExperiment.cpp			\
						RunState.cpp			\
						CComposite.cpp			\
						CScaler.cpp


noinst_HEADERS	=	CReadoutMain.h Skeleton.h CTCLAuthorizer.h	\
			CAuthorizedTclServer.h	CExperiment.h Asserts.h	\
			RunState.h CObject.h CComposite.h CScaler.h





INCLUDES	=	-I@top_srcdir@/base/exception	\
			-I@top_srcdir@/base/tclplus	\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/servers/portmanager	\
			-I@top_srcdir@/base/dataflow		\
			-I@top_srcdir@/daq/format		\
			$(TCL_FLAGS)

libSBSProductionReadout_la_LIBADD	=	@top_srcdir@/daq/format/libdataformat.la		\
						@top_srcdir@/base/exception/libException.la	\
						@top_srcdir@/base/tclplus/libtclPlus.la		\
						@top_srcdir@/servers/portmanager/libPortManager.la \
						@top_srcdir@/base/dataflow/libDataFlow.la


libSBSProductionReadout_la_LDFLAGS	=	-version-info	1:0:0	\
						$(TCL_LDFLAGS)

EXTRA_DIST	=	options.ggo Skeleton.cpp

noinst_PROGRAMS	= Readout tests

Readout_SOURCES = Skeleton.cpp

Readout_DEPENDENCIES = libSBSProductionReadout.la
Readout_LDADD	= libSBSProductionReadout.la	\
		  @top_srcdir@/base/exception/libException.la	\
		  @top_srcdir@/base/tclplus/libtclPlus.la


options.o:	options.c
	$(CC) -c -I. options.c

options.c:	options.ggo
	$(GENGETOPT) <options.ggo --file=options

clean-local:
	-rm -f options.c options.h

tests_SOURCES=TestRunner.cpp experimentTests.cpp \
		compositeTests.cpp

tests_LDADD = 	CExperiment.lo	\
		CComposite.lo	\
		RunState.lo	\
		@top_srcdir@/daq/format/libdataformat.la	\
		@top_srcdir@/base/dataflow/libDataFlow.la	\
		@top_srcdir@/base/exception/libException.la	\
		$(CPPUNIT_LDFLAGS)

tests_LDFLAGS=-Wl,"-rpath-link=$(libdir)"

TESTS=./tests

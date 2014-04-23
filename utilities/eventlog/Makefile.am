bin_PROGRAMS		=	eventlog

eventlog_SOURCES	=	eventlog.cpp eventlogMain.cpp

noinst_HEADERS		=	eventlogMain.h

eventlog_DEPENDENCIES	=	eventlogargs.o

BUILT_SOURCES		= 	eventlogargs.h

INCLUDES 		=	-I@top_srcdir@/base/headers		\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/base/os		\
				@OPENSSL_INCLUDES@

eventlog_LDADD		=	eventlogargs.o	\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@LIBEXCEPTION_LDFLAGS@			\
				@top_builddir@/base/os/libdaqshm.la		\
				$(THREADLD_FLAGS) @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@

eventlog_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

# Gengetopt stuff.

eventlogargs.h: eventlogargs.ggo
	$(GENGETOPT) <@top_srcdir@/utilities/eventlog/eventlogargs.ggo --file=eventlogargs
	$(CC) -c -I. eventlogargs.c


eventlogargs.o: eventlogargs.ggo
	$(GENGETOPT) <@top_srcdir@/utilities/eventlog/eventlogargs.ggo --file=eventlogargs
	$(CC) -c -I. eventlogargs.c

# Extra files to distribute.

EXTRA_DIST		=	eventlogargs.ggo eventlog.xml



#-------------------------------------------------------------
#
# Test programs.
#
noinst_PROGRAMS		=	fakeruns eventlogTests

fakeruns_SOURCES	= 	fakeruns.cpp

fakeruns_LDADD		=	@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/os/libdaqshm.la              \
				@LIBEXCEPTION_LDFLAGS@

eventlogTests_SOURCES = TestRunner.cpp eventlogTests.cpp
eventlogTests_CPPFLAGS=@CPPUNIT_CFLAGS@
eventlogTests_LDADD   = @CPPUNIT_LDFLAGS@ \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@LIBEXCEPTION_LDFLAGS@			\
				@top_builddir@/base/os/libdaqshm.la		\
				$(THREADLD_FLAGS)

TESTS=eventlogTests
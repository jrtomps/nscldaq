bin_PROGRAMS		=	eventlog

eventlog_SOURCES	=	eventlog.cpp eventlogMain.cpp

noinst_HEADERS		=	eventlogMain.h

eventlog_DEPENDENCIES	=	eventlogargs.o

BUILT_SOURCES		= 	eventlogargs.h

INCLUDES 		=	-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/base/os

eventlog_LDADD		=	eventlogargs.o	\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/exception/libException.la	\
				@top_builddir@/base/os/libdaqshm.la		\
				$(THREADLD_FLAGS)

eventlog_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

# Gengetopt stuff.

eventlogargs.h: eventlogargs.ggo
	$(GENGETOPT) <eventlogargs.ggo --file=eventlogargs
	$(CC) -c -I. eventlogargs.c


eventlogargs.o: eventlogargs.ggo
	$(GENGETOPT) <eventlogargs.ggo --file=eventlogargs
	$(CC) -c -I. eventlogargs.c

# Extra files to distribute.

EXTRA_DIST		=	eventlogargs.ggo eventlog.xml



#-------------------------------------------------------------
#
# Test programs.
#
noinst_PROGRAMS		=	fakeruns

fakeruns_SOURCES	= 	fakeruns.cpp

fakeruns_LDADD		=	@top_srcdir@/daq/format/libdataformat.la	\
				@top_srcdir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/os/libdaqshm.la              \
				@top_srcdir@/base/exception/libException.la

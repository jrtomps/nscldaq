bin_PROGRAMS		=	eventlog

eventlog_SOURCES	=	eventlog.cpp eventlogMain.cpp

noinst_HEADERS		= 	eventlogMain.h

eventlog_DEPENDENCIES	=	eventlogargs.o


INCLUDES 		=	-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow

eventlog_LDADD		=	eventlogargs.o	\
				@top_srcdir@/daq/format/libdataformat.la	\
				@top_srcdir@/base/dataflow/libDataFlow.la	\
				@top_srcdir@/base/exception/libException.la

# Gengetopt stuff.

eventlogargs.o: eventlogargs.ggo
	$(GENGETOPT) <eventlogargs.ggo --file=eventlogargs
	$(CC) -c -I. eventlogargs.c

# Extra files to distribute.

EXTRA_DIST		=	eventlogargs.ggo
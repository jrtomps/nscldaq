bin_PROGRAMS	=	dumper


dumper_SOURCES	=	CDataSource.cpp 	\
			CRingDataSource.cpp	\
			CFileDataSource.cpp	\
			BufdumpMain.cpp		\
			../StringsToIntegers.cpp	\
			dumper.cpp

dumper_DEPENDENCIES = 	dumperargs.o

noinst_HEADERS	=	CDataSource.h		\
			CRingDataSource.h	\
			CFileDataSource.h	\
			BufdumpMain.h		\
			../StringsToIntegers.h	\
			dumperargs.h



INCLUDES 		=	-I.. 					\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/uri			\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow

dumper_LDADD		=	dumperargs.o	\
				@top_srcdir@/daq/format/libdataformat.la	\
				@top_srcdir@/base/dataflow/libDataFlow.la	\
				@top_srcdir@/base/exception/libException.la


dumperargs.h: dumperargs.ggo
	$(GENGETOPT) <dumperargs.ggo --file=dumperargs
	$(CC) -c -I. dumperargs.c


dumperargs.o: dumperargs.ggo
	$(GENGETOPT) <dumperargs.ggo --file=dumperargs
	$(CC) -c -I. dumperargs.c


EXTRA_DIST=dumperargs.ggo
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
			../StringsToIntegers.h

BUILT_SOURCES	=	dumperargs.h dumperargs.c


INCLUDES 		=	-I.. 					\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/uri			\
				-I@top_srcdir@/base/exception		\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow

dumper_LDADD		=	dumperargs.o	\
				@top_srcdir@/base/uri/liburl.la			\
				@top_srcdir@/daq/format/libdataformat.la	\
				@top_srcdir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/uri/liburl.la	\
				@top_srcdir@/base/exception/libException.la	\
				$(THREADLD_FLAGS)

clean-local:
	rm -f dumperargs.h dumperargs.c dumperargs.o

dumper_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

dumperargs.h: dumperargs.ggo



dumperargs.c:
	$(GENGETOPT) <dumperargs.ggo --file=dumperargs --unamed-opts="OPTIONS"



dumperargs.o: dumperargs.c dumperargs.h
	$(GENGETOPT) <dumperargs.ggo --file=dumperargs --unamed-opts="OPTIONS"
	$(CC) -c -I. dumperargs.c $(THREADCFLAGS)


EXTRA_DIST=dumperargs.ggo

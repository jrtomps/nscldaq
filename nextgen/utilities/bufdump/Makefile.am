bin_PROGRAMS	=	dumper


dumper_SOURCES	=	BufdumpMain.cpp		\
			../StringsToIntegers.cpp	\
			dumper.cpp

dumper_DEPENDENCIES = 	dumperargs.o

noinst_HEADERS	=	BufdumpMain.h		\
			../StringsToIntegers.h

BUILT_SOURCES	=	dumperargs.h dumperargs.c


INCLUDES 		=	-I.. 					\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/uri			\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow		\
				-I@top_srcdir@/base/os

dumper_LDADD		=	dumperargs.o	\
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/uri/liburl.la	\
				@LIBEXCEPTION_LDFLAGS@			\
				@top_builddir@/base/os/libdaqshm.la            \
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


EXTRA_DIST=dumperargs.ggo dumper.xml


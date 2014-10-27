bin_PROGRAMS	=	dumper


dumper_SOURCES	=	BufdumpMain.cpp		\
			@srcdir@/../StringsToIntegers.cpp	\
			dumper.cpp @srcdir@/dumperargs.c

noinst_HEADERS	=	BufdumpMain.h		\
			@srcdir@/../StringsToIntegers.h

BUILT_SOURCES	=	@srcdir@/dumperargs.c


dumper_CPPFLAGS =	-I@srcdir@/.. 				\
				-I.. 					\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/uri			\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow		\
				-I@top_srcdir@/base/os

dumper_LDADD		=		\
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/uri/liburl.la	\
				@LIBEXCEPTION_LDFLAGS@				\
				@top_builddir@/base/os/libdaqshm.la            \
				$(THREADLD_FLAGS)

clean-local:
	rm -f @srcdir@/dumperargs.h  dumperargs.o $(BUILT_SOURCES)

dumper_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

@srcdir@/dumperargs.c: @srcdir@/dumperargs.h


@srcdir@/dumperargs.h: @srcdir@/dumperargs.ggo 
	$(GENGETOPT) < @srcdir@/dumperargs.ggo \
		--output-dir=@srcdir@ --file=dumperargs --unamed-opts="OPTIONS" \
		--set-version=@VERSION@


EXTRA_DIST=dumperargs.ggo dumper.xml


bin_PROGRAMS = teering

teering_SOURCES = teering.cpp cmdline.c ../fragment.c

BUILT_SOURCES=cmdline.c

noinst_HEADERS=cmdline.h

cmdline.h: cmdline.c

cmdline.c: options.ggo
	$(GENGETOPT) --input=options.ggo

INCLUDES = -I@top_srcdir@/daq/eventbuilder  \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os


teering_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libRingBuffer.la     \
	@top_builddir@/base/dataflow/libDataFlow.la	\
	@top_builddir@/base/os/libdaqshm.la		 \
	@top_builddir@/base/exception/libException.la \
	@THREADLD_FLAGS@

teering_LDFLAGS= -Wl,"-rpath=@libdir@"
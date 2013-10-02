bin_PROGRAMS = teering

teering_SOURCES = teering.cpp tcmdline.c ../fragment.c

BUILT_SOURCES=tcmdline.c

noinst_HEADERS=tcmdline.h

tcmdline.h: tcmdline.c

tcmdline.c: options.ggo
	$(GENGETOPT) --input=options.ggo -F tcmdline

INCLUDES = -I@top_srcdir@/daq/eventbuilder  \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os


teering_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libRingBuffer.la     \
	@top_builddir@/base/dataflow/libDataFlow.la	\
	@top_builddir@/base/os/libdaqshm.la		 \
	@LIBTCLPLUS_LDFLAGS@	\
	@THREADLD_FLAGS@

teering_LDFLAGS= -Wl,"-rpath=@libdir@"

EXTRA_DIST=options.ggo teering.xml

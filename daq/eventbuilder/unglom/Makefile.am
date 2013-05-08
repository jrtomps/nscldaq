bin_PROGRAMS=unglom

unglom_SOURCES=unglom.cpp cmdline.c

INCLUDES=-I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format       \
	-I@top_srcdir@/base/exception   \
	-I@top_srcdir@/base/os		\
	-I@top_srcdir@/base/headers  

unglom_LDADD = @top_builddir@/daq/format/libdataformat.la  \
	@top_builddir@/daq/eventbuilder/libEventBuilder.la \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@top_builddir@/base/exception/libException.la

BUILT_SOURCES = cmdline.c cmdline.h

cmdline.c: cmdline.h

cmdline.h: cmdline.ggo
	@GENGETOPT@ --input=cmdline.ggo

EXTRA_DIST=cmdline.ggo

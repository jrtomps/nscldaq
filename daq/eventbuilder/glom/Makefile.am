bin_PROGRAMS=glom

glom_SOURCES= @srcdir@/glom.c glomMain.cpp


BUILT_SOURCES = @srcdir@/glom.c

glom_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format          \
	-I@top_srcdir@/base/os

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
	@top_builddir@/daq/format/libdataformat.la                     \
	@top_builddir@/base/os/libdaqshm.la

@srcdir@/glom.c:  @srcdir@/glom.h

@srcdir@/glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@srcdir@ --file-name=glom

EXTRA_DIST=glom.ggo glom.xml

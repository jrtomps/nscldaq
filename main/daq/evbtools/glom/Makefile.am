bin_PROGRAMS=glom

glom_SOURCES= glomMain.cpp
nodist_glom_SOURCES=glom.c glom.h

BUILT_SOURCES = glom.c glom.h

glom_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format          \
	-I@top_srcdir@/base/os

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
	@top_builddir@/daq/format/libdataformat.la                     \
	@top_builddir@/base/os/libdaqshm.la

glom.c:  glom.h

glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@builddir@ --file-name=glom

EXTRA_DIST=glom.ggo glom.xml

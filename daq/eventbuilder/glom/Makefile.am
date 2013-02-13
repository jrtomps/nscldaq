bin_PROGRAMS=glom

glom_SOURCES= glom.c glomMain.cpp

noinst_HEADERS = glom.h

BUILT_SOURCES = glom.c glom.h

INCLUDES = -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format          \
	-I@top_srcdir@/base/os

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
	@top_builddir@/daq/format/libdataformat.la                     \
	@top_builddir@/base/os/libdaqshm.la

glom.c:  glom.h

glom.h: glom.ggo
	@GENGETOPT@ --input=glom.ggo --file-name=glom
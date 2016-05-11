bin_PROGRAMS=glom

<<<<<<< HEAD
glom_SOURCES= @srcdir@/glom.c glomMain.cpp


BUILT_SOURCES = @srcdir@/glom.c
=======
glom_SOURCES= glomMain.cpp
nodist_glom_SOURCES=glom.c glom.h

BUILT_SOURCES = glom.c glom.h
>>>>>>> master

glom_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format          \
	-I@top_srcdir@/base/os

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
	@top_builddir@/daq/format/libdataformat.la                     \
	@top_builddir@/base/os/libdaqshm.la

<<<<<<< HEAD
@srcdir@/glom.c:  @srcdir@/glom.h

@srcdir@/glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@srcdir@ --file-name=glom
=======
glom.c:  glom.h

glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@builddir@ --file-name=glom
>>>>>>> master

EXTRA_DIST=glom.ggo glom.xml

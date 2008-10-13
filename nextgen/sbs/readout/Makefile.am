lib_LTLIBRARIES	=	libSBSProductionReadout.la

libSBSProductionReadout_la_SOURCES	=	options.c \
						CReadoutMain.cpp


noinst_HEADERS	=	CReadoutMain.h Skeleton.h


INCLUDES	=	-I@top_srcdir@/base/exception	\
			-I@top_srcdir@/base/tclplus	\
			-I@top_srcdir@/base/headers	\
			$(TCL_FLAGS)

libSBSProductionReadout_la_LIBADD	=	@top_srcdir@/base/exception/libException.la	\
						@top_srcdir@/base/tclplus/libtclPlus.la

libSBSProductionReadout_la_LDFLAGS	=	-version-info	1:0:0	\
						$(TCL_LDFLAGS)

EXTRA_DIST	=	options.ggo Skeleton.cpp

noinst_PROGRAMS	= Readout

Readout_SOURCES = Skeleton.cpp

Readout_DEPENDENCIES = libSBSProductionReadout.la
Readout_LDADD	= libSBSProductionReadout.la	\
		  @top_srcdir@/base/exception/libException.la	\
		  @top_srcdir@/base/tclplus/libtclPlus.la


options.o:	options.c
	$(CC) -c -I. options.c

options.c:	options.ggo
	$(GENGETOPT) <options.ggo --file=options

clean-local:
	-rm -f options.c options.h

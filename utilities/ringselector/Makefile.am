bin_PROGRAMS =  ringselector

ringselector_SOURCES 	= 	ringselector.cpp RingSelectorMain.cpp ../StringsToIntegers.cpp

ringselector_DEPENDENCIES =	parser.o

noinst_HEADERS		=	 ../StringsToIntegers.h RingSelectorMain.h

BUILT_SOURCES	= parser.h

INCLUDES		= 	-I@top_srcdir@/base/exception	\
				-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/daq/format	\
			        -I@top_srcdir@/base/dataflow \
				-I@top_srcdir@/base/os

ringselector_LDADD	=  	parser.o 					\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/os/libdaqshm.la		\
				@top_builddir@/base/exception/libException.la	\
				$(THREADLD_FLAGS) 

ringselector_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)



#----------------------- Gengetopt stuff:

parser.h: parser.ggo
	$(GENGETOPT) <parser.ggo --file=parser
	$(CC)  -c parser.c

parser.o: parser.ggo
	$(GENGETOPT) <parser.ggo --file=parser 
	$(CC) -c parser.c

#----------------------- extra files to distribute.

EXTRA_DIST	=	ringselector.xml parser.ggo

#----------------------- Tests:
#  Since this is the first guy written to use it, the tests for
#  StringsToInteger are here too.
#

check_PROGRAMS	=	stringstointtests selectortests

stringstointtests_SOURCES	=	TestRunner.cpp str2int.cpp



stringstointtests_LDADD		= ../StringsToIntegers.lo	\
				-L$(libdir)			\
				$(CPPUNIT_LDFLAGS)		\
				@top_builddir@/base/exception/libException.la

stringstointtests_LDFLAGS	=	-Wl,"-rpath-link=$(libdir)"

selectortests_SOURCES	=	TestRunner.cpp selectortests.cpp

selectortests_LDADD	=	$(CPPUNIT_LDFLAGS)	\
				@top_builddir@/base/os/libdaqshm.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la			\
				@top_builddir@/base/exception/libException.la

selectortests_LDFLAGS	=	-Wl,"-rpath=link=$(libdir)"

selectortests_CXXFLAGS	=	-DBINDIR="\"$(bindir)\""


TESTS=./stringstointtests ./selectortests


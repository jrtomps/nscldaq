bin_PROGRAMS =  ringselector

ringselector_SOURCES 	= 	ringselector.cpp RingSelectorMain.cpp ../StringsToIntegers.cpp

ringselector_DEPENDENCIES =	parser.o

noinst_HEADERS		=	 ../StringsToIntegers.h RingSelectorMain.h

BUILT_SOURCES	= parser.h

INCLUDES		= 	-I@top_srcdir@/base/exception	\
				-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/daq/format	\
			        -I@top_srcdir@/base/dataflow

ringselector_LDADD	=  	parser.o 					\
				@top_srcdir@/daq/format/libdataformat.la	\
				@top_srcdir@/base/dataflow/libDataFlow.la	\
				@top_srcdir@/base/exception/libException.la



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
				-lException

stringstointtests_LDFLAGS	=	-Wl,"-rpath-link=$(libdir)"

selectortests_SOURCES	=	TestRunner.cpp selectortests.cpp

selectortests_LDADD	=	$(CPPUNIT_LDFLAGS)	\
				-L$(libdir) -ldataformat	\
				-lDataFlow			\
				-lException

selectortests_LDFLAGS	=	-Wl,"-rpath=link=$(libdir)"

selectortests_CXXFLAGS	=	-DBINDIR="\"$(bindir)\""


TESTS=./stringstointtests ./selectortests


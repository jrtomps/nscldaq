bin_PROGRAMS =  ringselector

ringselector_SOURCES 	= 	ringselector.cpp RingSelectorMain.cpp ../StringsToIntegers.cpp \
				RingBufferQueue.cpp COutputter.cpp

ringselector_DEPENDENCIES =	parser.o

noinst_HEADERS		=	 ../StringsToIntegers.h RingSelectorMain.h \
				RingBufferQueue.h COutputter.h

BUILT_SOURCES	= parser.h

ringselector_CPPFLAGS		= 	@LIBTCLPLUS_CFLAGS@	\
				-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/daq/format	\
			        -I@top_srcdir@/base/dataflow \
				-I@top_srcdir@/base/os	\
				-I@top_srcdir@/base/thread

ringselector_LDADD	=  	parser.o 					\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/os/libdaqshm.la		\
				@top_builddir@/base/thread/libdaqthreads.la     \
				@LIBEXCEPTION_LDFLAGS@				\
				$(THREADLD_FLAGS) 

ringselector_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)



#----------------------- Gengetopt stuff:

parser.h: parser.ggo
	$(GENGETOPT) <@top_srcdir@/utilities/ringselector/parser.ggo --file=parser
	$(CC)  -c parser.c

parser.o: parser.ggo
	$(GENGETOPT) <@top_srcdir@/utilities/ringselector/parser.ggo --file=parser 
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
				@LIBEXCEPTION_LDFLAGS@

stringstointtests_LDFLAGS	=	-Wl,"-rpath-link=$(libdir)"

selectortests_SOURCES	=	TestRunner.cpp selectortests.cpp

selectortests_LDADD	=	$(CPPUNIT_LDFLAGS)	\
				@top_builddir@/base/os/libdaqshm.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la			\
				@LIBEXCEPTION_LDFLAGS@

selectortests_LDFLAGS	=	-Wl,"-rpath=link=$(libdir)"

selectortests_CXXFLAGS	=	-DBINDIR="\"$(bindir)\""


TESTS=./stringstointtests ./selectortests


bin_PROGRAMS    = ringFragmentSource
noinst_PROGRAMS = ringTestSource
lib_LTLIBRARIES= libTimeExtractorTest.la libS800TimeExtractor.la libGretinaTimeExtractor.la

libTimeExtractorTest_la_SOURCES = testTimeExtractor.c	

libS800TimeExtractor_la_SOURCES = s800timestamp.c

libGretinaTimeExtractor_la_SOURCES = gretinaTimestamp.c


ringFragmentSource_SOURCES=CRingSource.cpp Main.cpp

ringTestSource_SOURCES= ringTestSource.cpp ringtestoptions.c TestSource.cpp

noinst_HEADERS=CRingSource.h s800.h TestSource.h


BUILT_SOURCES = cmdline.c GetOpt.cpp ringtestoptions.c

#
#  This is a bit funky.  We copy the options.ggo skeleton file and add to it
#  options needed for the ring buffer...then generate cmdline.{h,c}

cmdline.c: ../options.ggo
	cp ../options.ggo .
	echo option \"ring\" r \"URL of ring from which fragments come\" string required >> options.ggo
	echo option \"timestampextractor\" t \"Shared library with timestamp extraction code\" string required >>options.ggo
	$(GENGETOPT) --input=options.ggo 

GetOpt.cpp: ../GetOpt.cpp ../GetOpt.h
	cp ../GetOpt.{cpp,h} .


ringtestoptions.h: ringtestoptions.c

ringtestoptions.c: ringtestoptions.ggo
	$(GENGETOPT) --input=ringtestoptions.ggo --file=ringtestoptions

ringTestSource_LDADD = \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/exception/libException.la \
	-ldl 

.FIRST: 
	rm -rf $(BUILT_SOURCES)

clean-local:
	-rm -rf $(BUILT_SOURCES)

INCLUDES=-I.. -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/exception -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

ringFragmentSource_LDADD=  		 \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/dataflow/libRingBuffer.la \
	@top_builddir@/base/os/libdaqshm.la	\
	GetOpt.lo cmdline.lo \
	$(THREAD_LDFLAGS) 

EXTRA_DIST=ringsource.xml ringtestoptions.ggo

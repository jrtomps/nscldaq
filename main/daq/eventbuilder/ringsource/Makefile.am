bin_PROGRAMS    = ringFragmentSource
noinst_PROGRAMS = ringTestSource
lib_LTLIBRARIES= libTimeExtractorTest.la libS800TimeExtractor.la libGretinaTimeExtractor.la

libTimeExtractorTest_la_SOURCES = testTimeExtractor.c	
libTimeExtractorTest_la_CPPFLAGS=-I@top_srcdir@/daq/format

libS800TimeExtractor_la_SOURCES = s800timestamp.c
libS800TimeExtractor_la_CPPFLAGS=-I@top_srcdir@/daq/format

libGretinaTimeExtractor_la_SOURCES = gretinaTimestamp.c
libGretinaTimeExtractor_la_CPPFLAGS=-I@top_srcdir@/daq/format


ringFragmentSource_SOURCES=@srcdir@/rfcmdline.c CRingSource.cpp Main.cpp GetOpt.cpp

ringTestSource_SOURCES= ringTestSource.cpp ringtestoptions.c TestSource.cpp

ringTestSource_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format -I@top_srcdir@/base/os

noinst_HEADERS=CRingSource.h s800.h TestSource.h


BUILT_SOURCES = @srcdir@/rfcmdline.c @srcdir@/rfcmdline.h  \
	@srcdir@/ringtestoptions.c @srcdir@/ringtestoptions.h @srcdir@/options.ggo

#
#  This is a bit funky.  We copy the options.ggo skeleton file and add to it
#  options needed for the ring buffer...then generate cmdline.{h,c}

@srcdir@/options.ggo: @top_srcdir@/daq/eventbuilder/options.ggo
	cp @top_srcdir@/daq/eventbuilder/options.ggo  @srcdir@
	echo option \"ring\" r \"URL of ring from which fragments come\" string required >> @srcdir@/options.ggo
	echo option \"timestampextractor\" t \"Shared library with timestamp extraction code\" string optional >> \
		@srcdir@/options.ggo
	echo option \"expectbodyheaders\" e \"Body headers are expected on every ring item.\" flag off >> \
		@srcdir@/options.ggo
	echo option \"oneshot\" o \"One shot after n end run items\" optional  int default=\"1\" >> @srcdir@/options.ggo
	echo option \"timeout\" T \"Timeout waiting for end runs in oneshot mode\" int default=\"10\" optional  >>@srcdir@/options.ggo

@srcdir@/rfcmdline.c: @srcdir@/rfcmdline.h


@srcdir@/rfcmdline.h: @srcdir@/options.ggo
	$(GENGETOPT) --output-dir=@srcdir@ \
		--input=@srcdir@/options.ggo -F rfcmdline

@srcdir@/ringtestoptions.c: @srcdir@/ringtestoptions.h


@srcdir@/ringtestoptions.h: @srcdir@/ringtestoptions.ggo
	$(GENGETOPT) --input=@srcdir@/ringtestoptions.ggo \
		--output-dir=@srcdir@ --file=ringtestoptions

ringTestSource_LDADD = \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBEXCEPTION_LDFLAGS@	\
	-ldl 

.FIRST: 
	rm -rf $(BUILT_SOURCES)

clean-local:
	-rm -rf $(BUILT_SOURCES) @srcdir@/$(BUILT_SOURCES)

ringFragmentSource_CPPFLAGS=-I@top_srcdir@/daq/eventbuilder -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format \
	@LIBTCLPLUS_CFLAGS@	 \
	 -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os

ringFragmentSource_LDADD=  		 \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/dataflow/libRingBuffer.la \
	@top_builddir@/base/os/libdaqshm.la	\
	$(THREAD_LDFLAGS)  -ldl

EXTRA_DIST=ringsource.xml ringtestoptions.ggo

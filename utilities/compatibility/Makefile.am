bin_PROGRAMS = compatibilitybuffer compatibilitylogger BufferToRing convert10to11

noinst_HEADERS = buffer.h buftypes.h CBufferConverter.h CLogRun.h \
    OldDataFormat.h



compatibilitybuffer_SOURCES = compatibilitybuffer.cpp CBufferConverter.cpp \
	@srcdir@/parser.c

EXTRA_DIST=compatibilitybuffer.ggo loggeroptions.ggo eventlog-compat.in \
	spectcldaq.in spectcldaq.server.tcl compatibility.xml s800toring.in \
	netcat.tcl buffertoring.ggo test.ggo


compatibilitybuffer_LDADD = \
	@top_builddir@/daq/format/libdataformat.la
compatibilitybuffer_CPPFLAGS = -I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os

compatibilitylogger_SOURCES = compatibilitylogger.cpp CLogRun.cpp \
	@srcdir@/loggeroptions.c
compatibilitylogger_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os



BufferToRing_SOURCES = BufferToRing.cpp btoroptions.c
BufferToRing_LDADD   = @top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/os/libdaqshm.la
BufferToRing_LDFLAGS = -ldl
BufferToRing_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os -I@top_srcdir@/daq/eventbuilder


convert10to11_SOURCES = Convert10to11.cpp

convert10to11_LDADD = @top_builddir@/daq/format/libdataformat.la \
    @top_builddir@/base/os/libdaqshm.la \
                @top_builddir@/utilities/IO/libdaqio.la
convert10to11_CPPFLAGS=-I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os \
        -I@top_srcdir@/utilities/IO


#  Anything that is generated from a .in file is in builddir anything
#  else is in srcdir:

install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) @builddir@/eventlog-compat $(bindir)
	$(INSTALL_SCRIPT) @builddir@/spectcldaq $(bindir)
	$(INSTALL_SCRIPT) @srcdir@/spectcldaq.server.tcl  $(bindir)/spectcldaq.server
	$(INSTALL_SCRIPT) @builddir@/s800toring  $(bindir)
	$(INSTALL_SCRIPT) @srcdir@/netcat.tcl $(bindir)
	$(INSTALL_SCRIPT) @builddir@/bufdump $(bindir)


#----------------------------------------------------------------

#  This stuff is needed to be sure that gengetopt builds the stuff 
#  we need prior to application compilations.


BUILT_SOURCES = @srcdir@/parser.c @srcdir@/loggeroptions.c \
	@srcdir@/btoroptions.c @srcdir@/btoroptions.h @srcdir@/test.c @srcdir@/test.h

@srcdir@/parser.c: @srcdir@/parser.h

@srcdir@/parser.h: @srcdir@/compatibilitybuffer.ggo
	$(GENGETOPT) <@srcdir@/compatibilitybuffer.ggo --file=parser \
		--output-dir=@srcdir@


@srcdir@/loggeroptions.c: @srcdir@/loggeroptions.h

@srcdir@/loggeroptions.h: @srcdir@/loggeroptions.ggo
	$(GENGETOPT) <@srcdir@/loggeroptions.ggo --file=loggeroptions \
		--output-dir=@srcdir@


@srcdir@/btoroptions.c: @srcdir@/btoroptions.h

@srcdir@/btoroptions.h: @srcdir@/buffertoring.ggo
	$(GENGETOPT) <@srcdir@/buffertoring.ggo --file=btoroptions \
		--output-dir=@srcdir@


#------------------------------------------------------------------------
# For testing:
#

noinst_PROGRAMS = testDataSource

testDataSource_SOURCES = testDataSource.cpp @srcdir@/test.c

testDataSource_CPPFLAGS = -I@top_srcdir@/base/os


testDataSource_LDADD    = @top_builddir@/base/os/libdaqshm.la

test.c: test.h

test.h: @srcdir@/test.ggo
	$(GENGETOPT) <@srcdir@/test.ggo --file=test \
		--output-dir=@srcdir@

bin_PROGRAMS = compatibilitybuffer compatibilitylogger BufferToRing convert10to11

noinst_HEADERS = buffer.h buftypes.h CBufferConverter.h CLogRun.h \
    OldDataFormat.h

compatibilitybuffer_SOURCES = compatibilitybuffer.cpp CBufferConverter.cpp \
	@srcdir@/parser.c

EXTRA_DIST=compatibilitybuffer.ggo loggeroptions.ggo eventlog-compat.in \
	spectcldaq.in spectcldaq.server.tcl compatibility.xml s800toring.in \
	netcat.tcl


compatibilitybuffer_LDADD = \
	@top_builddir@/daq/format/libdataformat.la

INCLUDES = -I@top_srcdir@/daq/format -I@top_srcdir@/base/headers \
	-I@top_srcdir@/base/os

compatibilitylogger_SOURCES = compatibilitylogger.cpp CLogRun.cpp \
	@srcdir@/loggeroptions.c



BufferToRing_SOURCES = BufferToRing.cpp
BufferToRing_LDADD   = @top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/os/libdaqshm.la

convert10to11_SOURCES = Convert10to11.cpp

convert10to11_LDADD = @top_builddir@/daq/format/libdataformat.la \
    @top_builddir@/base/os/libdaqshm.la


#  Anything that is generated from a .in file is in builddir anything
#  else is in srcdir:

install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) @builddir@/eventlog-compat $(bindir)
	$(INSTALL_SCRIPT) @builddir@/spectcldaq $(bindir)
	$(INSTALL_SCRIPT) @srcdir@/spectcldaq.server.tcl  $(bindir)/spectcldaq.server
	$(INSTALL_SCRIPT) @builddir@/s800toring  $(bindir)
	$(INSTALL_SCRIPT) @srcdir@/netcat.tcl $(bindir)


#----------------------------------------------------------------

#  This stuff is needed to be sure that gengetopt builds the stuff 
#  we need prior to application compilations.


BUILT_SOURCES = @srcdir@/parser.c @srcdir@/loggeroptions.c

@srcdir@/parser.c: @srcdir@/parser.h

@srcdir@/parser.h: @srcdir@/compatibilitybuffer.ggo
	$(GENGETOPT) <@srcdir@/compatibilitybuffer.ggo --file=parser \
		--output-dir=@srcdir@


@srcdir@/loggeroptions.c: @srcdir@/loggeroptions.h

@srcdir@/loggeroptions.h: @srcdir@/loggeroptions.ggo
	$(GENGETOPT) <@srcdir@/loggeroptions.ggo --file=loggeroptions \
		--output-dir=@srcdir@



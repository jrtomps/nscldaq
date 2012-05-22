bin_PROGRAMS = compatibilitybuffer compatibilitylogger BufferToRing

noinst_HEADERS = buffer.h buftypes.h CBufferConverter.h CLogRun.h

compatibilitybuffer_SOURCES = compatibilitybuffer.cpp CBufferConverter.cpp

EXTRA_DIST=compatibilitybuffer.ggo loggeroptions.ggo eventlog-compat.in \
	spectcldaq.in spectcldaq.server.tcl compatibility.xml s800toring.in \
	netcat.tcl


compatibilitybuffer_LDADD = parser.o

INCLUDES = -I@top_srcdir@/daq/format -I@top_srcdir@/base/headers

compatibilitylogger_SOURCES = compatibilitylogger.cpp CLogRun.cpp

compatibilitylogger_LDADD = loggeroptions.o

BufferToRing_SOURCES = BufferToRing.cpp
BufferToRing_LDADD   = @top_srcdir@/daq/format/libdataformat.la

install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) eventlog-compat $(bindir)
	$(INSTALL_SCRIPT) spectcldaq $(bindir)
	$(INSTALL_SCRIPT) spectcldaq.server.tcl  $(bindir)/spectcldaq.server
	$(INSTALL_SCRIPT) s800toring  $(bindir)
	$(INSTALL_SCRIPT) netcat.tcl $(bindir)


#----------------------------------------------------------------

#  This stuff is needed to be sure that gengetopt builds the stuff 
#  we need prior to application compilations.

compatibilitybuffer_DEPENDENCIES =  parser.o

BUILT_SOURCES = parser.h loggeroptions.h

parser.h: compatibilitybuffer.ggo
	$(GENGETOPT) <compatibilitybuffer.ggo --file=parser
	$(CC) -c parser.c

parser.o: compatibilitybuffer.ggo
	$(GENGETOPT) <compatibilitybuffer.ggo --file=parser
	$(CC) -c parser.c


compatibilitylogger_DEPENDENCIES = loggeroptions.o

loggeroptions.h: loggeroptions.ggo
	$(GENGETOPT) <loggeroptions.ggo --file=loggeroptions
	$(CC) -c loggeroptions.c

loggeroptions.o: loggeroptions.ggo
	$(GENGETOPT) <loggeroptions.ggo --file=loggeroptions
	$(CC) -c loggeroptions.c



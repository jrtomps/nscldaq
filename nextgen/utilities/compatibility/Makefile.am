bin_PROGRAMS = compatibilitybuffer compatibilitylogger

noinst_HEADERS = buffer.h buftypes.h CBufferConverter.h

compatibilitybuffer_SOURCES = compatibilitybuffer.cpp CBufferConverter.cpp

EXTRA_DIST=compatibilitybuffer.ggo compatibilitybuffer.xml

compatibilitybuffer_LDADD = parser.o

INCLUDES = -I@top_srcdir@/daq/format -I@top_srcdir@/base/headers

compatibilitylogger_SOURCES = compatibilitylogger.cpp CLogRun.cpp

compatibilitylogger_LDADD = loggeroptions.o

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


loggeroptions_DEPENDENCIESE = loggeroptions.o

loggeroptions.h: loggeroptions.ggo
	$(GENGETOPT) <loggeroptions.ggo --file=loggeroptions
	$(CC) -c loggeroptions.c

loggeroptions.o: loggeroptions.ggo
	$(GENGETOPT) <loggeroptions.ggo --file=loggeroptions
	$(CC) -c loggeroptions.c



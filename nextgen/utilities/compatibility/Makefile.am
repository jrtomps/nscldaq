bin_PROGRAMS = compatibilitybuffer

noinst_HEADERS = buffer.h buftypes.h CBufferConverter.h

compatibilitybuffer_SOURCES = compatibilitybuffer.cpp CBufferConverter.cpp

EXTRA_DIST=compatibilitybuffer.ggo compatibilitybuffer.xml

compatibilitybuffer_LDADD = parser.o

INCLUDES = -I@top_srcdir@/daq/format -I@top_srcdir@/base/headers

#----------------------------------------------------------------

#  This stuff is needed to be sure that gengetopt builds the stuff 
#  we need prior to application compilations.

compatibilitybuffer_DEPENDENCIES =  parser.o

BUILT_SOURCES = parser.h

parser.h: compatibilitybuffer.ggo
	$(GENGETOPT) <compatibilitybuffer.ggo --file=parser
	$(CC) -c parser.c

parser.o: compatibilitybuffer.ggo
	$(GENGETOPT) <compatibilitybuffer.ggo --file=parser
	$(CC) -c parser.c


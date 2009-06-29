
H_FILES = DAQDisplayXML.h DAQDisplayText.h DAQDisplayText_H.h \
DAQDisplayProto.h DAQDisplayHTML.h DAQDisplayHTML_H.h DAQConfigParser.h \
DAQConfigParser_H.h DAQServConfig.h DAQCParserOut.h DAQCommunicatorOut.h \
DAQOstreamOut.h

FILES = DAQDisplayXML DAQDisplayText DAQDisplayHTML DAQDisplayText_H \
DAQDisplayHTML_H DAQConfigParser DAQConfigParser_H DAQServConfig \
DAQCParserOut DAQCommunicatorOut DAQOstreamOut

include ../../../config.make

all: include lib prog

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend

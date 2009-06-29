
SUB_DIRS = 

FILES = DAQROCNode DAQFilterNode DAQBufferGetPage \
DAQObjStat DAQThreadStat DAQSinkRQ DAQLinkStat \
DAQSinkDeleteRQ DAQBufferAccept DAQLinkMgr DAQUsedPagesStat \
DAQDebugLevel DAQPRQStat DAQServStat DAQFreePagesStat \
DAQLogState

H_FILES = DAQROCNode.h DAQFilterNode.h DAQBufferGetPage.h \
DAQObjStat.h DAQThreadStat.h DAQSinkRQ.h DAQLinkStat.h \
DAQSinkDeleteRQ.h DAQBufferAccept.h DAQLinkMgr.h DAQUsedPagesStat.h \
DAQDebugLevel.h DAQPRQStat.h DAQServStat.h DAQFreePagesStat.h \
DAQLogState.h

include ../../config.make

all: include lib prog

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)

lib: $(O_FILES)
	$(AR) $(CLIENTLIB) $(O_FILES)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_CLIENTLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend

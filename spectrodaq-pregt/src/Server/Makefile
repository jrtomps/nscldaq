
SUB_DIRS = 

FILES = DAQServNode DAQBufferGetPage_H DAQObjStat_H \
DAQThreadStat_H DAQSinkRQ_H DAQLinkThreadMgr \
DAQLinkRQ_H DAQLinkRQ DAQLinkRQThread DAQLinkDeleteRQ_H \
DAQLinkDeleteRQ DAQLinkStat_H DAQLinkInbThread \
DAQLinkOutbThread DAQSinkRQThread DAQSinkDeleteRQ_H \
DAQBufferAccept_H DAQDeliveryThread DAQUsedPagesStat_H \
DAQDebugLevel_H DAQGetPageThread DAQPRQStat_H DAQServStat_H \
DAQLogState_H DAQScavengerThread DAQFreePagesStat_H 

H_FILES = DAQServNode.h DAQBufferGetPage_H.h DAQObjStat_H.h \
DAQThreadStat_H.h DAQSinkRQ_H.h DAQLinkThreadMgr.h \
DAQLinkRQ_H.h DAQLinkRQ.h DAQLinkRQThread.h DAQLinkDeleteRQ_H.h \
DAQLinkDeleteRQ.h DAQLinkStat_H.h DAQLinkInbThread.h \
DAQLinkOutbThread.h DAQLinkThread.h DAQSinkRQThread.h \
DAQSinkDeleteRQ_H.h DAQBufferAccept_H.h DAQAcceptTimeOut.h \
DAQDeliveryThread.h DAQUsedPagesStat_H.h DAQDebugLevel_H.h \
DAQServClock.h DAQGetPageThread.h DAQPRQStat_H.h \
DAQServStat_H.h DAQFreePagesStat_H.h DAQLogState_H.h DAQScavengerThread.h

include ../../config.make

all: include lib prog

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)

lib: $(O_FILES)
	$(AR) $(SERVLIB) $(O_FILES)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_SERVLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend

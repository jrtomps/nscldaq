
SUB_DIRS = 

FILES = DAQServNode DAQBufferGetPage_H DAQObjStat_H \
DAQThreadStat_H DAQSinkRQ_H DAQLinkThreadMgr \
DAQLinkRQ_H DAQLinkRQ DAQLinkRQThread DAQLinkDeleteRQ_H \
DAQLinkDeleteRQ DAQLinkStat_H DAQLinkInbThread \
DAQLinkOutbThread DAQSinkRQThread DAQSinkDeleteRQ_H \
DAQBufferAccept_H DAQDeliveryThread DAQUsedPagesStat_H \
DAQDebugLevel_H DAQGetPageThread DAQPRQStat_H DAQServStat_H \
DAQInterfaceThread DAQInterfaceMgr DAQInterfaceICmd \
DAQInterpCmd_tsstat DAQInterpCmd_usedpages DAQInterpCmd_linkstat \
DAQInterpCmd_debuglevel DAQInterpCmd_prqstat DAQInterpCmd_servstat \
DAQInterpCmd_objstat DAQFreePagesStat_H DAQInterpCmd_freepages \
DAQLogState_H DAQScavengerThread

H_FILES = DAQServNode.h DAQBufferGetPage_H.h DAQObjStat_H.h \
DAQThreadStat_H.h DAQSinkRQ_H.h DAQLinkThreadMgr.h \
DAQLinkRQ_H.h DAQLinkRQ.h DAQLinkRQThread.h DAQLinkDeleteRQ_H.h \
DAQLinkDeleteRQ.h DAQLinkStat_H.h DAQLinkInbThread.h \
DAQLinkOutbThread.h DAQLinkThread.h DAQSinkRQThread.h \
DAQSinkDeleteRQ_H.h DAQBufferAccept_H.h DAQAcceptTimeOut.h \
DAQDeliveryThread.h DAQUsedPagesStat_H.h DAQDebugLevel_H.h \
DAQServClock.h DAQGetPageThread.h DAQPRQStat_H.h \
DAQServStat_H.h DAQInterfaceThread.h DAQInterfaceMgr.h \
DAQCIfaceObj.h DAQInterfaceICmd.h DAQInterpCmd_tsstat.h \
DAQInterpCmd_usedpages.h DAQInterpCmd_linkstat.h \
DAQInterpCmd_debuglevel.h DAQInterpCmd_prqstat.h DAQInterpCmd_servstat.h \
DAQInterpCmd_objstat.h DAQFreePagesStat_H.h DAQInterpCmd_freepages.h \
DAQLogState_H.h DAQScavengerThread.h

include ../../config.make

all: include lib prog

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
#	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib: $(O_FILES)
	$(AR) $(SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog::
#	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_SERVLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make install); done;

clean::
	rm -f *.o
#	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
#	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend

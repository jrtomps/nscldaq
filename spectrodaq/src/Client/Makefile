
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
#	for i in $(SUB_DIRS); do (cd $$i; make include); done;

lib: $(O_FILES)
	$(AR) $(CLIENTLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

prog::
#	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_CLIENTLIB) $(O_FILES)
#	for i in $(SUB_DIRS); do (cd $$i; make install); done;

clean::
	rm -f *.o
#	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
#	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;

include .depend

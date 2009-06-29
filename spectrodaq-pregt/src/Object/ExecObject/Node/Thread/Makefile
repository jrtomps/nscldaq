
H_FILES = DAQThreadId.h DAQThread.h DAQThreadFactory.h DAQThreadMutex.h \
DAQThreadCond.h DAQThreadHashTable.h DAQThreadDebugMutex.h \
DAQThreadRecursiveMutex.h

FILES = daqthreadjoin DAQThreadId DAQThread DAQThreadFactory \
DAQThreadMutex DAQThreadCond DAQThreadDebugMutex \
DAQThreadRecursiveMutex

include ../../../../../config.make

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

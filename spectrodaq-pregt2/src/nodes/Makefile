
SUB_DIRS = 

H_FILES = DAQDispatcher.h DAQExecFactory.h DAQExecId.h DAQExecObject.h \
DAQNodeFactory.h DAQNode.h DAQNodeId.h DAQThreadCond.h DAQThreadFactory.h \
DAQThread.h DAQThreadHashTable.h DAQThreadId.h DAQThreadMutex.h \
DAQThreadRecursiveMutex.h DAQExecTrack.h DAQExecTrackEntry.h

FILES = DAQDispatcher DAQExecFactory DAQExecId DAQExecObject DAQNode \
DAQNodeFactory DAQNodeId DAQThread DAQThreadCond DAQThreadFactory \
DAQThreadId daqthreadjoin DAQThreadMutex DAQThreadRecursiveMutex \
DAQExecTrack 

include ../../config.make

all: include lib prog

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

prog::
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

clean:
	rm -f *.o 
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

realclean: clean
	rm -f .depend
	:> .depend
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	rm -f $(DAQLIB)

include .depend

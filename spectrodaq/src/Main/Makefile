
H_FILES = DAQLogger.h DAQTrackMap.h mainexterns.h DAQMain.h DAQTrackMemory.h \
maintypes.h daqtypes.h maindefs.h DAQAlloc.h

FILES = DAQAlloc DAQLogger DAQMain DAQTrackMemory DAQTrackMap main

include ../../config.make

all: include lib

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)

prog::

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend

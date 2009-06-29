
SUB_DIRS = 

H_FILES = DAQCommInfo.h DAQCommMsgIface.h DAQCommMsg.h DAQCommunicator.h

FILES = DAQCommInfo DAQCommMsgIface DAQCommMsg DAQCommunicator

include ../../../config.make

all: include lib prog

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)

clean:
	rm -f *.o 

realclean: clean
	rm -f .depend
	:> .depend

include .depend

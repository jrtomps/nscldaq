
H_FILES = DAQAlarmMsg.h DAQAlarm.h DAQTrap.h

FILES = DAQAlarmMsg DAQAlarm DAQTrap

include ../../../config.make

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


FILES = daqpthreads

H_FILES = daqpthreads.h 

include ../../../config.make

all: include lib

include::
	$(DIRINST) $(DAQTHREADSINCDIR)
	$(INSTALL) $(H_FILES) $(DAQTHREADSINCDIR)

lib: $(O_FILES)
	$(AR) $(DAQTHREADSLIB) $(O_FILES)

install: $(O_FILES)
	$(DIRINST) $(INST_DAQTHREADSINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQTHREADSINCDIR)
	$(AR) $(INST_DAQTHREADSLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend

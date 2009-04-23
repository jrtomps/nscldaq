
FILES = hashtable xmlparse

H_FILES = hashtable.h xmlparse.h xmlalloc.h

include ../../config.make

all: include lib

pre::

include::
	$(DIRINST) $(XMLINCDIR)
	$(INSTALL) $(H_FILES) $(XMLINCDIR)

lib: $(O_FILES)
	$(AR) $(XMLPARSELIB) $(O_FILES)

install: $(O_FILES)
	$(DIRINST) $(INST_XMLINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_XMLINCDIR)
	$(AR) $(INST_XMLPARSELIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend	

include .depend

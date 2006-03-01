FILES =

H_FILES = 

SUB_DIRS = 

include ../../config.make

all: man

man: install

lib::

prog::

include:: 
	$(INSTALL) ./man1/*.1 $(DAQMANDIR)/man1
	$(DIRINST) $(USAGEINCDIR)
	$(INSTALL) ./usage/*.h $(USAGEINCDIR)

install::
	$(DIRINST) $(INST_DAQMANDIR)/man1
	$(INSTALL) ./man1/*.1 $(INST_DAQMANDIR)/man1
	$(DIRINST) $(INST_USAGEINCDIR)
	$(INSTALL) ./usage/*.h $(INST_USAGEINCDIR)

clean:

realclean: clean


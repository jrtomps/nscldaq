FILES =

H_FILES = 

SUB_DIRS = 

include ../config.make

all:: 

lib::

include::

prog::

doc::
	cd $(TOPDIR)/doc 
	$(DOXYGEN) ./Doxyfile

install::
	$(DIRINST) $(INST_DAQDOCDIR)
	$(DIRINST) $(INST_DAQDOCDIR)/GettingStarted
	$(DIRINST) $(INST_DAQDOCDIR)/images
	$(DIRINST) $(INST_DAQDOCDIR)/html
	$(DIRINST) $(INST_DAQDOCDIR)/man/man3
	$(INSTALL) Images/*.gif $(INST_DAQDOCDIR)/images
	$(INSTALL) GettingStarted/*.html $(INST_DAQDOCDIR)/GettingStarted
	$(INSTALL) doxydoc/html/* $(INST_DAQDOCDIR)/html
	$(INSTALL) doxydoc/man/man3/* $(INST_DAQDOCDIR)/man/man3

docclean:
	rm -rf $(TOPDIR)/doc/doxydoc/latex
	rm -rf $(TOPDIR)/doc/doxydoc/html
	rm -rf $(TOPDIR)/doc/doxydoc/man

clean:

realclean: clean


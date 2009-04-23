
FILES = xmlrole xmltok

H_FILES = asciitab.h iasciitab.h latin1tab.h nametab.h utf8tab.h xmldef.h \
xmlrole.h xmltok.h xmltok_impl.h ascii.h

include ../../config.make

all: include lib

pre::

nametab.h::
	rm -f $@
	../gennmtab/gennmtab >$@

include: nametab.h
	$(DIRINST) $(XMLINCDIR)
	$(INSTALL) $(H_FILES) $(XMLINCDIR)

lib: $(O_FILES)
	$(AR) $(XMLTOKLIB) $(O_FILES)

install: $(O_FILES)
	$(DIRINST) $(INST_XMLINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_XMLINCDIR)
	$(AR) $(INST_XMLTOKLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend	

include .depend

EXTRA_DIST = alltsts.tcl docbuilder.tcl \
	fragmenttest.tcl mansectests.tcl \
	metadirtest.tcl walkdirtest.tcl



install-exec-local: daq.pdf daq.mans
	$(mkinstalldirs) $(prefix)/share
	$(mkinstalldirs)  $(prefix)/share/man
	for i in 1 2 3 4 5 6 7 8; do $(mkinstalldirs) $(prefix)/share/man/man$$i; done
	- for i in 1 2 3 4 5 6 7 8; do for f in *.$$i*; do  $(INSTALL_DATA) $$f $(prefix)/share/man/man$$i; done; done
	$(INSTALL_DATA) daq.pdf $(prefix)/share


if PDFDOCS
DOCBOOK2PDF = docbook2pdf
else
DOCBOOK2PDF = touch daq.pdf 
endif


if XMLMANS
DOCBOOK2MAN = xmlto man
else
DOCBOOK2MAN = touch
endif

daq.pdf: daq.xml
	$(DOCBOOK2PDF) daq.xml



daq.mans: daq.xml
	- $(DOCBOOK2MAN) daq.xml
	touch daq.mans

daq.xml:
	$(TCLSH_CMD) docbuilder.tcl @top_srcdir@/docconfig "NSCL DAQ Software Documentation" @top_srcdir@ >daq.xml



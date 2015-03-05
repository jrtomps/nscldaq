EXTRA_DIST = alltsts.tcl docbuilder.tcl \
	fragmenttest.tcl mansectests.tcl \
	metadirtest.tcl walkdirtest.tcl \
	tutorial.xml 

DIST_SUBDIRS = images



install-exec-local: daq.html daq.mans tutorial.pdf tutorial.html
	$(mkinstalldirs) @prefix@/share
	$(mkinstalldirs)  @prefix@/share/man
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/tuthtml
	for i in 1 2 3 4 5 6 7 8; do $(mkinstalldirs) @prefix@/share/man/man$$i; done
	- for i in 1 2 3 4 5 6 7 8; do for f in *.$$i*; do  $(INSTALL_DATA) $$f @prefix@/share/man/man$$i; done; done
	- $(INSTALL_DATA) html/* @prefix@/share/html
	- $(mkinstalldirs) @prefix@/share/images @prefix@/share/images/callouts
	- $(INSTALL_DATA) @srcdir@/images/*.gif @prefix@/share/images;
	- $(INSTALL_DATA) @srcdir@/images/callouts/*.gif  @prefix@/share/images/callouts
	- $(INSTALL_DATA) tutorial.pdf @prefix@/share
	- $(INSTALL_DATA) tuthtml/*   @prefix@/share/tuthtml



clean-local:
	-rm -f *.pdf
	-rm -f *.[0-9]*
	-rm -f daq.xml
	-rm -f daq.html daq.mans tutorial.pdf tutorial.html



if PDFDOCS
DOCBOOK2PDF = docbook2pdf
DOCBOOK2HTML= docbook2html 
else
DOCBOOK2PDF = touch daq.pdf 
DOCBOOK2HTML= touch daq.html
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

daq.html: daq.xml
	- rm -rf html
	mkdir html
	- $(DOCBOOK2HTML) -o @builddir@/html daq.xml
	touch daq.html

daq.xml:
	$(TCLSH_CMD) @srcdir@/docbuilder.tcl @top_srcdir@/docconfig "NSCL DAQ Software Documentation" @top_srcdir@ >daq.xml


tutorial.pdf: @srcdir@/tutorial.xml
	$(DOCBOOK2PDF) @srcdir@/tutorial.xml

tutorial.html: @srcdir@/tutorial.xml
	- rm -rf tuthtml
	mkdir @builddir@/tuthtml
	- $(DOCBOOK2HTML) -o @builddir@/tuthtml @srcdir@/tutorial.xml
	touch tutorial.html

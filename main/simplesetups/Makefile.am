#Gets built into the doc tree of nscldaq.


DOCS    = @srcdir@/v775setup.xml @srcdir@/v775book.xml
FIGURES = @srcdir@/v775ediagram.jpeg @srcdir@/v775ediagram.eps
FIGSRC  = @srcdir/v775ediagram.dia

EXTRA_DIST=$(DOCS)

install-exec-local: v775book.pdf
	$(mkinstalldirs) @prefix/share/html
	$(mkinstalldirs) @prefix/share/pdfs
	$(INSTALL_DATA) $(FIGURES) @prefix@/share/html
	$(INSTALL_DATA) v775book.pdf @prefix@/share/pdfs

v775book.pdf: v775book.xml v775setup.xml
	pwd
	docbook2pdf v775book.xml

clean-local:
	rm -f v775book.pdf v775setup.pdf

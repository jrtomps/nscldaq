scripts = loadshaper.tcl n568b.tcl \
	shaper8.ui.tcl shaper16.ui.tcl shaperapp.tcl \
	shaperapp16.tcl

samples = goShaper goShaper16 \
	myshaper.cfg shaper.cfg


EXTRA_DIST = $(scripts) $(samples)


install-exec-local:
	$(mkinstalldirs) $(prefix)/contrib $(prefix)/contrib/N568Panel
	$(mkinstalldirs) $(prefix)/contrib/N568Panel/examples
	for f in $(scripts);  \
	do \
	   $(INSTALL_PROGRAM) $(srcdir)/$$f $(prefix)/contrib/N568Panel; \
        done
	for f in $(samples); \
	do			\
	   $(INSTALL_PROGRAM) $(srcdir)/$$f $(prefix)/contrib/N568Panel/examples ; \
	done 
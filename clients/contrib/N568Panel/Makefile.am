isntall-exec-local:
	$(mkinstalldirs) $(prefix)/contrib
	$(mkinstalldirs) $(prefix)/N568Panel
	$(INSTALL_PROGRAM) $(srcdir) *.tcl  $(prefix)/N568Panel
	$(INSTALL_PROGRAM) $(srcdir) goShaper goShaper16 $(prefix)/N568Panel
	$(INSTALL_DATA)   $(srcdir) *.cfg                $(prefix)/N568Panel

EXTRA_DIST = loadshaper.tcl n568b.tcl shaper.ui.tcl \
	shaperapp.tcl shaperapp16.tcl shaper16.ui.tcl myshaper.cfg shaper.cfg \
	goShaper goShaper16
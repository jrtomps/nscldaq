scripts = vhqpanel.ui.tcl vhqapp.tcl

EXTRA_DIST=$(scripts) 

install-exec-local:
	$(mkinstalldirs) $(prefix)/contrib $(prefix)/contrib/vhqpanel
	$(mkinstalldirs) $(prefix)/contrib/N568Panel/examples
	for f in $(scripts);  \
	do \
	   $(INSTALL_PROGRAM) $(srcdir)/$$f $(prefix)/contrib/vhqpanel; \
        done
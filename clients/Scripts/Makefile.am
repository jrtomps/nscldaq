install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(INSTALL_SCRIPT) cratelocator.tcl $(prefix)/Scripts
	rm -f $(bindir)/cratelocator
	$(LN_S) $(prefix)/Scripts/cratelocator.tcl $(bindir)/cratelocator

EXTRA_DIST = cratelocator.tcl
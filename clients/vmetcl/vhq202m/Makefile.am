install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(INSTALL_DATA)  vhq202m.tcl $(prefix)/Scripts

EXTRA_DIST = vhq202m.tcl AAReadme.txt

install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(INSTALL_DATA) AAReadme.txt $(prefix)/Docs/vmetcl/vhq202m.txt
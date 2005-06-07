install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(prefix)/etc
	$(INSTALL_DATA)  vhq202m.tcl $(prefix)/Scripts

EXTRA_DIST = vhq202m.tcl AAReadme.txt vhq.3tcl

install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(INSTALL_DATA) AAReadme.txt $(prefix)/Docs/vmetcl/vhq202m.txt

man_MANS = vhq.3tcl
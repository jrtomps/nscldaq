install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(prefix)/etc
	$(INSTALL_DATA)  camac.tcl $(prefix)/Scripts
	$(INSTALL_DATA)  wienercamac.tcl $(prefix)/Scripts
install-exec-local:
EXTRA_DIST = camac.tcl AAReadme.txt  wienercamac.tcl  cescamac.3tcl


install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(INSTALL_DATA) AAReadme.txt $(prefix)/Docs/vmetcl/camactcl.txt

man_MANS = cescamac.3tcl 

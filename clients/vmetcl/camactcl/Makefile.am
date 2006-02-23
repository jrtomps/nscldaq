APPDIR=$(prefix)/bin
PKGDIR=$(prefix)/TclLibs/camac
DOCDIR=$(prefix)/Docs/vmetcl

install-exec-local:
	$(mkinstalldirs) $(PKGDIR)
	$(INSTALL_SCRIPT)  camac.tcl $(PKGDIR)
	$(INSTALL_SCRIPT)  wienercamac.tcl $(PKGDIR)
	echo "pkg_mkIndex $(PKGDIR) *.tcl" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

EXTRA_DIST = camac.tcl AAReadme.txt  wienercamac.tcl  cescamac.3tcl


install-data-local:
	$(mkinstalldirs) $(DOCDIR)
	$(INSTALL_DATA) AAReadme.txt $(DOCDIR)

man_MANS = cescamac.3tcl 

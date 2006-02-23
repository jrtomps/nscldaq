install-exec-local:
	$(mkinstalldirs) $(prefix)/TclLibs/vhq
	$(INSTALL_DATA)  vhq202m.tcl $(prefix)/TclLibs/vhq
	echo "pkg_mkIndex $(prefix)/TclLibs/vhq *.tcl" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

EXTRA_DIST = vhq202m.tcl AAReadme.txt vhq.3tcl

install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(INSTALL_DATA) AAReadme.txt $(prefix)/Docs/vmetcl/vhq202m.txt

man_MANS = vhq.3tcl
SUBDIRS =	base servers utilities docbuild docconfig

install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
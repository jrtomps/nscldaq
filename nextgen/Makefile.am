SUBDIRS =	base/CopyrightTools base/exception base/tclplus servers base  utilities docbuild docconfig

install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
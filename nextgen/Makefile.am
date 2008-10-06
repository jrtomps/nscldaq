SUBDIRS =	base/CopyrightTools base/exception base/tclplus servers base  \
		daq utilities docbuild docconfig usb sbs


install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

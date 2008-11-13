if BUILD_SBS_DRIVER
  SBSDIRS=sbs
endif

SUBDIRS =	base/CopyrightTools base/exception base/tclplus servers base  \
		daq utilities docbuild docconfig usb $(SBSDIRS)


install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

EXTRA_DIST=dotests

TESTS=./dotests

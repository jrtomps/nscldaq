if BUILD_SBS_DRIVER
  SBSDIRS=sbs
endif

SUBDIRS =	base/CopyrightTools base/exception base/tclplus servers base  \
		daq utilities docbuild docconfig usb $(SBSDIRS)


install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/include
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	$(INSTALL_SCRIPT) config.h @prefix@/include

EXTRA_DIST=dotests

TESTS=./dotests

DRIVERDest=@prefix@/TclLibs/vmusbdrivers

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	$(INSTALL_SCRIPT) @srcdir@/*driver.tcl $(DRIVERDest)
	$(INSTALL_SCRIPT) @srcdir@/VMUSBDriverSupport.tcl $(DRIVERDest)
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#

EXTRA_DIST=sis330xdriver.tcl VMUSBDriverSupport.tcl

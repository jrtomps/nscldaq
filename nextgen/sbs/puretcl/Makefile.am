EXTRA_DIST  = 	camac.tcl	\
		wienercamac.tcl

PACKAGE	=	@prefix@/TclLibs/sbspuretcl

install-exec-local:
	$(mkinstalldirs) $(PACKAGE)
	$(INSTALL_SCRIPT) *.tcl $(PACKAGE)
	echo package ifneeded camac 1.1 [list source [file join \$$dir camac.tcl]] > $(PACKAGE)/pkgIndex.tcl
	echo package ifneeded wienercamac 1.1 [list source [file join \$$dir wienercamac.tcl]] >> $(PACKAGE)/pkgIndex.tcl


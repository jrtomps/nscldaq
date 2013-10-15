SUBDIRS=cv6533

PACKAGES=client.tcl configfile.tcl gdgcontrol.tcl gdgwidget.tcl 

APPS=gdgpanel.tcl v812usbcontrol.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/vmusbcontrol
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/vmusbcontrol; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/vmusbcontrol "*.tcl" | tclsh
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@; done
	$(INSTALL_SCRIPT) @srcdir@/v812usbcontrol.tcl @bindir@/v895usbcontrol.tcl	



EXTRA_DIST= $(APPS) $(PACKAGES)
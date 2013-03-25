SUBDIRS=cv6533

PACKAGES=client.tcl configfile.tcl gdgcontrol.tcl gdgwidget.tcl 

APPS=gdgpanel.tcl v812usbcontrol.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/vmusbcontrol
	$(INSTALL_SCRIPT) $(PACKAGES) @prefix@/TclLibs/vmusbcontrol
	echo pkg_mkIndex -verbose @prefix@/TclLibs/vmusbcontrol "*.tcl" | tclsh
	$(INSTALL_SCRIPT) $(APPS) @bindir@
	$(INSTALL_SCRIPT) v812usbcontrol.tcl @bindir@/v895usbcontrol.tcl
	


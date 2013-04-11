PACKAGES=channel.tcl channelParams.tcl v6533.tcl
APP=v6533panel.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/v6533
	echo pkg_mkIndex -verbose @prefix@/TclLibs/v6533 "*.tcl" | tclsh	
	$(INSTALL_SCRIPT) $(PACKAGES) @prefix@/TclLibs/v6533
	$(INSTALL_SCRIPT) $(APP) @bindir@

EXTRA_DIST=$(APP) $(PACKAGES)
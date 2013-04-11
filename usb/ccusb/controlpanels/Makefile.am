PACKAGES=ledcontrol.tcl

APPS=ledph7106.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/ccusbpanels @bindir@
	$(INSTALL_SCRIPT) $(PACKAGES) @prefix@/TclLibs/ccusbpanels
	echo pkg_mkIndex @prefix@/TclLibs/ccusbpanels "*.tcl" | @TCLSH_CMD@
	$(INSTALL_SCRIPT) $(APPS)     @bindir@

EXTRA_DIST=$(APPS) $(PACKAGES)


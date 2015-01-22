PACKAGES=ledcontrol.tcl

APPS=ledph7106.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/ccusbpanels @bindir@
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/ccusbpanels; done
	echo pkg_mkIndex @prefix@/TclLibs/ccusbpanels "*.tcl" | @TCLSH_CMD@
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f     @bindir@; done

EXTRA_DIST=$(APPS) $(PACKAGES)


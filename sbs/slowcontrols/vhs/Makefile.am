DRIVERDIR = $(prefix)/TclLibs/vhs
APPDIR=$(prefix)/bin


PACKAGES = pollmanager.tcl vhs.tcl VhsWidgets.tcl
APPS     = vhsPanel.tcl

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	$(INSTALL_SCRIPT) $(PACKAGES) $(DRIVERDIR)
	for f in $(APPS); do $(INSTALL_SCRIPT) $$f $(APPDIR)/`basename $$f .tcl`; done
	echo "pkg_mkIndex $(DRIVERDIR) *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl


EXTRA_DIST= $(PACKAGES) $(APPS)  $(HELPFILES) vhs.xml

TCLPACKAGES=burnpackage.tcl TopGUi.ui.tcl dvdburnoutput.ui.tcl
TCLAPPS=dvdburn burngui

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/dvdburn
	$(INSTALL_SCRIPT) $(TCLPACKAGES) $(prefix)/TclLibs/dvdburn
	$(INSTALL_SCRIPT) $(TCLAPPS)      $(prefix)/bin
	echo "pkg_mkIndex $(prefix)/TclLibs/dvdburn *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl


EXTRA_DIST = $(TCLAPPS) $(TCLPACKAGES)
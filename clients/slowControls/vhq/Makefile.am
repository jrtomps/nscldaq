DRIVERDIR=$(prefix)/TclLibs/vhq
APPDIR=$(prefix)/bin
HELPDIR=$(prefix)/TclLibs/data/vhq

VERSION=2.0			#Must match the package versions!!!

PACKAGES=vhqPanel.tcl
APPS=vhqControl.tcl
MANPAGES=vhqControl.1 vhqPanel.3 vhqconfig.5

HELPFILES= configfile.html configilimit.html configmenu.html \
    configspeeds.html fileclose.html filemenu.html filenew.html \
    filequit.html fileread.html filewrite.html gui.html \
    helpmenu.html menubar.html overview.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	$(INSTALL_SCRIPT) $(PACKAGES) $(DRIVERDIR)
	for f in $(APPS); do $(INSTALL_SCRIPT) $$f $(APPDIR)/`basename $$f .tcl`; done
	$(INSTALL_DATA)   $(HELPFILES) $(HELPDIR)
	echo "pkg_mkIndex $(DRIVERDIR) *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl


EXTRA_DIST= $(PACKAGES) $(APPS) $(MANPAGES) $(HELPFILES)

man_MANS = $(MANPAGES)

if EPICS

HELPFILES=add.html alarmconfig.html channelmenu.html \
	config.html filemenu.html gui.html \
	helpmenu.html overview.html

HELPDIR=$(prefix)/TclLibs/data/epicsdisplay

PKGDIR=$(prefix)/TclLibs/epicsdisplay


install-exec-local:
	$(mkinstalldirs)   $(bindir)
	$(mkinstalldirs)   $(HELPDIR)
	$(mkinstalldirs)   $(PKGDIR)
	$(INSTALL_SCRIPT)  displayepics.tcl $(PKGDIR)
	$(INSTALL_SCRIPT)  EpicsDisplay.tcl $(bindir)
	$(INSTALL_SCRIPT)  epicsdisplay     $(bindir)
	$(INSTALL_DATA)    $(HELPFILES) $(HELPDIR)


EXTRA_DIST = channels.txt  EpicsDisplay.tcl displayepics.tcl \
	epicsdisplay.xml epicsdisplay.in \
	$(HELPFILES)

endif
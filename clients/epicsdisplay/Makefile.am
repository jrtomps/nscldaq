HELPFILES=add.html alarmconfig.html channelmenu.html \
	config.html filemenu.html gui.html \
	helpmenu.html overview.html
HELPDIR=$(prefix)/Scripts/epicsdisplayHelp

install-exec-local:
	$(mkinstalldirs)   $(prefix)/Scripts $(HELPDIR)
	$(INSTALL_SCRIPT)  displayepics.tcl $(prefix)/Scripts
	$(INSTALL_SCRIPT)  EpicsDisplay.tcl $(prefix)/Scripts
	$(INSTALL_SCRIPT)  epicsdisplay     $(bindir)
	$(INSTALL_DATA)    $(HELPFILES) $(HELPDIR)

man_MANS = EpicsDisplay.1

EXTRA_DIST = channels.txt  EpicsDisplay.tcl displayepics.tcl EpicsDisplay.1 epicsdisplay.in \
	$(HELPFILES)

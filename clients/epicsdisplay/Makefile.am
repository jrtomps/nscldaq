install-exec-local:
	$(mkinstalldirs)   $(prefix)/Scripts
	$(INSTALL_SCRIPT)  displayepics.tcl $(prefix)/Scripts
	$(INSTALL_SCRIPT)  EpicsDisplay.tcl $(prefix)/Scripts
	$(INSTALL_SCRIPT)  epicsdisplay     $(bindir)

man_MANS = EpicsDisplay.1

EXTRA_DIST = channels.txt  EpicsDisplay.tcl displayepics.tcl EpicsDisplay.1 epicsdisplay.in

install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(INSTALL_SCRIPT) *.tcl $(prefix)/Scripts
	$(INSTALL_SCRIPT) burngui dvdburn $(prefix)/Scripts
	rm -f $(bindir)/burngui $(bindir)/dvdburn
	$(LN_S) $(prefix)/Scripts/burngui $(bindir)/burngui
	$(LN_S) $(prefix)/Scripts/dvdburn $(bindir)/dvdburn


EXTRA_DIST = TopGUi.ui.tcl burngui burnpackage.tcl dvdburn dvdburnoutput.ui.tcl
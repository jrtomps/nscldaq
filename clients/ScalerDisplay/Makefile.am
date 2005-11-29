install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) *.tcl  $(prefix)/Scripts
	$(INSTALL_SCRIPT) ScalerDisplay $(prefix)/bin


EXTRA_DIST = ScalerDisplay.tcl scaler.tcl \
 ScalerDisplay.in scalerdisplay.1

man_MANS = scalerdisplay.1
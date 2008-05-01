install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(mkinstalldirs) $(prefix)/TclLibs/ScalerDisplay
	$(INSTALL_SCRIPT) $(TCLPACKAGES) $(prefix)/TclLibs/ScalerDisplay
	$(INSTALL_SCRIPT) ScalerDisplay ScalerDisplay.tcl $(bindir)


TCLPACKAGES=scaler.tcl 
TCLAPPS=ScalerDisplay ScalerDisplay.tcl

EXTRA_DIST = ScalerDisplay.tcl scaler.tcl \
 ScalerDisplay.in scalerdisplay.xml


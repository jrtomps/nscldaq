
DRIVERDest = @prefix@/TclLibs/mesytec

TCLDRIVERS = MCFD16USB.tcl MCFD16RC.tcl MCFD16Gui.tcl MCFD16CommandLogger.tcl \
						 MCFD16Memorizer.tcl
TCLAPPS = MCFD16Control.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/bin/`basename $$f .tcl`  && \
		chmod a+x @prefix@/bin/`basename $$f .tcl` ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
TCLTESTS=tclTests.tcl mcfd16usb.test mcfd16gui.test mcfd16rc.test mcfd16commandlogger.test \
				 mcfd16memorizer.test


check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) $(TCLTESTS) $(TCLAPPS)

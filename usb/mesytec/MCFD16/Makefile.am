
DRIVERDest = @prefix@/TclLibs/mesytec

TCLDRIVERS = MCFD16USB.tcl MCFD16RC.tcl MCFD16Gui.tcl
TCLAPPS = MCFD16Control.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	$(INSTALL_SCRIPT) @srcdir@/MCFD16Control.tcl @prefix@/bin/MCFD16Control  && \
		chmod a+x @prefix@/bin/MCFD16Control
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
TCLTESTS=tclTests.tcl mcfd16usb.test mcfd16gui.test mcfd16rc.test


check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) $(TCLTESTS)

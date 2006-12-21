TCLPACKAGES=sequencerGui.tcl controller.tcl pkgIndex.tcl
EXAMPLEFILES=ReadoutCallouts.tcl sequencerActions.tcl sequencer.config run.plan

EXTRA_DIST=$(TCLPACKAGES) $(EXAMPLEFILES) sequencer.xml \
install-exec-local:
	$(mkinstalldirs) $(prefix)/TclLibs/sequencer
	$(mkinstalldirs) $(prefix)/examples/sequencer
	$(INSTALL_SCRIPT) $(TCLPACKAGES) $(prefix)/TclLibs/sequencer
	$(INSTALL_SCRIPT) $(EXAMPLEFILES) $(prefix)/examples/sequencer


sequencer.3tcl: sequencer.xml
	$(MANDOCBOOK) man sequencer.xml

man_MANS=sequencer.3tcl

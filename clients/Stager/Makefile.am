install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	for f in *.tcl *.sh; do  $(INSTALL_SCRIPT) $$f $(prefix)/Scripts;done
	./installpkg $(prefix)
	rm -f $(bindir)/Stager
	$(LN_S) $(prefix)/Scripts/StagerGui.tcl $(bindir)/Stager

EXTRA_DIST =  Experiment.tcl \
	GetTapeDrive.tcl \
	KeeplistGui.tcl \
	OS.tcl \
	ReadoutControl.tcl \
	ReadoutGui.tcl \
	ReadoutShell.tcl \
	RemoteInfo.tcl \
	Retain.tcl \
	RunReadout.sh \
	SetStagePolicy.tcl \
	StagePolicy.tcl \
	StagePolicyFree.tcl \
	StagePolicyUsed.tcl \
	Stager.tcl \
	StagerGui.tcl \
	directories.tcl \
	rsh.tcl \
	warning.tcl \
	installpkg
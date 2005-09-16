install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	for f in *.tcl *.sh; do  $(INSTALL_SCRIPT) $$f $(prefix)/Scripts;done
	export LD_LIBRARY_PATH=$(prefix)/lib
	export TCLLIBPATH="$(prefix)/lib $(prefix)/Scripts"
	echo package provide InstallRoot 1.0       > InstallRoot.tcl
	echo namespace eval InstallRoot {         >> InstallRoot.tcl
	echo  "variable root $(prefix)"             >> InstallRoot.tcl
	echo  proc Where {} {                     >> InstallRoot.tcl
	echo     variable root             >> InstallRoot.tcl
	echo     "return \$$root"             >> InstallRoot.tcl
	echo  }                                   >> InstallRoot.tcl
	echo }                                    >> InstallRoot.tcl
	$(INSTALL_SCRIPT) InstallRoot.tcl $(prefix)/Scripts
	./installpkg $(prefix)
	rm -f $(bindir)/Stager
	$(LN_S) $(prefix)/Scripts/StagerGui.tcl $(bindir)/Stager
	./installpkg $(prefix)


EXTRA_DIST = Configuration.tcl \
	DAQParameters.tcl \
	ExpFileSystemConfig.tcl \
	Experiment.tcl \
	GetTapeDrive.tcl \
	InitializeConfiguration.tcl \
	KeeplistGui.tcl \
	OS.tcl \
	ReadougGUIPanel.tcl \
	ReadougGUIPanel.ui \
	ReadougGUIPanel_ui.tcl \
	ReadoutControl.tcl \
	ReadoutGui.tcl \
	ReadoutShell.tcl \
	ReadoutState.tcl \
	RemoteInfo.tcl \
	Retain.tcl \
	RunReadout.sh \
	ScalerParameterGUI.tcl \
	SetStagePolicy.tcl \
	StagePolicy.tcl \
	StagePolicyFree.tcl \
	StagePolicyUsed.tcl \
	Stager.tcl \
	StagerConfiguration.tcl \
	StagerGui.tcl \
	UniqueInstance.tcl \
	build.sh \
	daqparameters.test \
	directories.tcl \
	directories.test \
	experiment.test \
	filesystem.test \
	initconfig.test \
	installpkg \
	installroot.test \
	keeplistDialog.tcl \
	keeplistDialog.ui \
	keeplistDialog_ui.tcl \
	keeplisttest.tk \
	ostest.test \
	readoutcontrol.test \
	readoutguitest.tk \
	retain.test \
	rsh.tcl \
	scalerParameterDialog.tcl \
	scalerParameterDialog.ui \
	scalerParameterDialog_ui.tcl \
	scalerparametergui.tk \
	selectReadout.tcl \
	selectReadout.ui \
	selectReadoutDialog.tcl \
	selectReadout_ui.tcl \
	selectreadoutest.tk \
	stagerconfiguration.test \
	stagertests.test \
	system.test \
	testrunner.tk \
	tests.test \
	warning.tcl 


bin_PROGRAMS =  SecretTk

SecretTk_SOURCES = SecretTk.c

SecretTk_LDFLAGS= $(TCL_LDFLAGS) $(X11LIBS)


lib_LTLIBRARIES = libWait.la
libWait_la_SOURCES = WaitPackage.cpp

libWait_la_LDFLAGS =   -L@top_srcdir@/TCL \
	-L@top_srcdir@/Exception -L@top_srcdir@/CopyrightTools \
	-version-info $(SOVERSION):0

	  
libWait_la_LIBADD =  -lTclPlus -lException  \
	$(TCL_LDFLAGS)	$(X11LIBS)  -lLicense  \
	-lm -lstdc++ -lgcc -lc

INCLUDES = -I@top_srcdir@/Headers \
	-I@top_srcdir@/TCL \
	-I@top_srcdir@/CopyrightTools \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"
install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(INSTALL_SCRIPT) $(TCLPACKAGES) $(prefix)/TclLibs/Stager
	$(INSTALL_SCRIPT) InstallRoot.tcl $(prefix)/TclLibs/Stager
	$(INSTALL_SCRIPT) ReadoutShell.tcl $(prefix)/bin
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) $$f  $(prefix)/bin/`basename $$f .tcl`; done
	$(INSTALL_SCRIPT) StagerGui.tcl $(prefix)/bin/Stager
	$(INSTALL_SCRIPT) RunReadout.sh  $(prefix)/TclLibs
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" > installpkg.tcl
	$(TCLSH_CMD) <installpkg.tcl
	echo Adjusting installation location of libWait.so compiled tcl extension.
	mv $(prefix)/lib/libWait.* $(prefix)/TclLibs

TCLPACKAGES=Configuration.tcl \
	DAQParameters.tcl \
	ExpFileSystemConfig.tcl \
	Experiment.tcl \
	GetTapeDrive.tcl \
	InitializeConfiguration.tcl \
	KeeplistGui.tcl \
	OS.tcl          \
	ReadougGUIPanel.tcl ReadougGUIPanel_ui.tcl \
	ReadoutControl.tcl \
	ReadoutGui.tcl \
	ReadoutState.tcl \
	RemoteInfo.tcl    \
	Retain.tcl  \
	ScalerParameterGUI.tcl \
	SetStagePolicy.tcl  \
	StagePolicy.tcl  \
	StagePolicyFree.tcl \
	StagePolicyUsed.tcl \
	Stager.tcl          \
	StagerConfiguration.tcl \
	UniqueInstance.tcl \
	directories.tcl \
	keeplistDialog.tcl keeplistDialog_ui.tcl \
	rsh.tcl \
	scalerParameterDialog.tcl scalerParameterDialog_ui.tcl \
	selectReadout.tcl selectReadout_ui.tcl \
	selectReadoutDialog.tcl \
	warning.tcl \
	bells.tcl \
	RunTime.tcl

TCLAPPS=ReadoutShell \
	StagerGui.tcl

TCLTESTS = daqparameters.test directories.test experiment.test filesystem.test \
	initconfig.test installroot.test keeplisttest.tk ostest.test readoutcontrol.test \
	readoutguitest.tk retain.test scalerparametergui.tk \
	selectreadoutest.tk stagerconfiguration.test stagertests.test system.test \
	testrunner.tk tests.test

EXTRA_DIST = $(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS) \
	ReadoutShell.tcl \
	ReadougGUIPanel.ui \
	Retain.tcl \
	RunReadout.sh \
	keeplistDialog.ui \
	scalerParameterDialog.ui \
	selectReadout.ui \
	warning.tcl InstallRoot.tcl.in


bin_PROGRAMS =  SecretTk

SecretTk_SOURCES = SecretTk.c

SecretTk_LDFLAGS= $(TCL_LDFLAGS) $(X11LIBS)


lib_LTLIBRARIES = libWait.la
libWait_la_SOURCES = WaitPackage.cpp

libWait_la_LDFLAGS =   -L@top_srcdir@/TCL \
	-L@top_srcdir@/Exception -L@top_srcdir@/CopyrightTools \
	-version-info $(SOVERSION):0

libWait_la_LIBADD =  -ltclPlus -lException  \
	$(TCL_LDFLAGS)	$(X11LIBS)  -lLicense  \
	-lm -lstdc++ -lgcc -lc

INCLUDES = -I@top_srcdir@/Headers \
	-I@top_srcdir@/TCL \
	-I@top_srcdir@/CopyrightTools \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"
install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	$(INSTALL_SCRIPT) @builddir@/InstallRoot.tcl $(prefix)/TclLibs/Stager
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(prefix)/bin/`basename $$f .tcl`; done
	$(INSTALL_SCRIPT) @srcdir@/ReadoutShell.tcl $(prefix)/bin
	$(INSTALL_SCRIPT) @srcdir@/RunReadout.sh  $(prefix)/TclLibs
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)
	echo Adjusting installation location of libWait.so compiled tcl extension.
	mv $(prefix)/lib/libWait.* $(prefix)/TclLibs
	$(INSTALL_DATA) @srcdir@/RunState.jpg @srcdir@/RunState.eps @prefix@/share/html

TCLPACKAGES=Configuration.tcl \
	DAQParameters.tcl \
	ExpFileSystemConfig.tcl \
	Experiment.tcl \
	InitializeConfiguration.tcl \
	OS.tcl          \
	ReadougGUIPanel.tcl ReadougGUIPanel_ui.tcl \
	ReadoutControl.tcl \
	ReadoutGui.tcl \
	ReadoutState.tcl \
	RemoteInfo.tcl    \
	ScalerParameterGUI.tcl \
	UniqueInstance.tcl \
	directories.tcl \
	rsh.tcl \
	scalerParameterDialog.tcl scalerParameterDialog_ui.tcl \
	selectReadout.tcl selectReadout_ui.tcl \
	selectReadoutDialog.tcl \
	warning.tcl \
	bells.tcl   \
	Monitor.tcl \
	RunTime.tcl

TCLAPPS=ReadoutShell


TCLTESTS = daqparameters.test directories.test experiment.test filesystem.test \
	initconfig.test installroot.test  ostest.test readoutcontrol.test \
	readoutguitest.tk  scalerparametergui.tk \
	selectreadoutest.tk  system.test \
	testrunner.tk tests.test

EXTRA_DIST = $(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS) \
	ReadoutShell.tcl   \
	ReadougGUIPanel.ui \
	RunReadout.sh \
	scalerParameterDialog.ui \
	selectReadout.ui \
	warning.tcl InstallRoot.tcl.in \
	readoutgui.xml RunState.eps RunState.jpg


bin_PROGRAMS =  SecretTk

SecretTk_SOURCES = SecretTk.c

SecretTk_LDFLAGS= $(TCL_LDFLAGS) $(X11LIBS)


lib_LTLIBRARIES = libWait.la
libWait_la_SOURCES = WaitPackage.cpp

libWait_la_LDFLAGS =	@LIBTCLPLUS_LDFLAGS@	\
			@top_builddir@/base/CopyrightTools/libLicense.la	 \
			-version-info $(SOVERSION)

libWait_la_LIBADD =  @LIBTCLPLUS_LDFLAGS@ \
	$(TCL_LDFLAGS)	$(X11LIBS)  -lLicense  \
	-lm -lstdc++ -lgcc -lc

INCLUDES =	-I@top_srcdir@/base/headers	\
		@LIBTCLPLUS_CFLAGS@ \
		-I@top_srcdir@/base/CopyrightTools \
		$(TCL_FLAGS) -DHOME=\"$(prefix)\"
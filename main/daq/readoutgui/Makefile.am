install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	$(INSTALL_SCRIPT) @builddir@/InstallRoot.tcl $(prefix)/TclLibs/Stager
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(prefix)/bin/`basename $$f .tcl`; done
	$(INSTALL_SCRIPT) @srcdir@/RunReadout.sh  $(prefix)/TclLibs
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)
	echo Adjusting installation location of libWait.so compiled tcl extension.
	mv $(prefix)/lib/libWait.* $(prefix)/TclLibs
	$(INSTALL_DATA) @srcdir@/RunState.jpg @srcdir@/RunState.eps @prefix@/share/html
	$(INSTALL_DATA) @srcdir@/readoutshell.jpg @srcdir@/readoutshell.eps @prefix@/share/html
	$(INSTALL_DATA) @srcdir@/eventlogsettings.jpg @srcdir@/eventlogsettings.eps @prefix@/share/html
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs *.so" | $(TCLSH_CMD)
	$(INSTALL_SCRIPT) ReadoutShell.tcl @bindir@/ReadoutShell
	$(INSTALL_SCRIPT) @srcdir@/start.bash @bindir@/start.bash


TCLPACKAGES=Configuration.tcl \
	DAQParameters.tcl \
	ExpFileSystemConfig.tcl \
	InitializeConfiguration.tcl \
	OS.tcl          \
	UniqueInstance.tcl \
	directories.tcl \
	rsh.tcl \
	warning.tcl \
	bells.tcl   \
	DataSourceManager.tcl \
	RunstateMachine.tcl \
	rdoCalloutsBundle.tcl \
	eventLogBundle.tcl \
	DataSourceUI.tcl     \
	ui.tcl	\
	sshProvider.tcl sshPrompt.tcl \
	 s800Provider.tcl \
	ReadoutGui.tcl \
	state.tcl \
	DataSourceMonitor.tcl \
	remoteControl.tcl \
	ReadoutGuiClient.tcl readoutGuiProvider.tcl \
  versionUtils.tcl error.png \
  delayProvider.tcl \
  offlineProvider.tcl \
	multilogger.tcl \
	stageareaValidation.tcl



TCLTESTS =    filesystem.test \
	initconfig.test installroot.test system.test tests.test  ostest.test \
	sshpipe.test s800.test  rstatemachine.test rdocallouts.test \
	  state.test versionutils.test delayprovider.test


TKTESTS =	readoutguitest.tk  scalerparametergui.tk \
	selectreadoutest.tk  tclTests.tcl \
	testrunner.tk

#
#  Note that A=1234 below is for the state.test restore-6

check-TESTS:
	A=1234 TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl
	@@ echo "readoutcontrol.test and tests.test load packages that produce output on stderr"
	@@ echo "This is the source of the "Test files exiting with errors"  message."


EXTRA_DIST = $(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS) $(TKTESTS)\
	RunReadout.sh \
	warning.tcl InstallRoot.tcl.in \
	readoutgui_user.xml RunState.eps RunState.jpg s800fake.tcl\
	readoutshell.jpg readoutshell.eps \
	sshsimulator.testscript start.bash \
	eventlogsettings.jpg eventlogsettings.eps error.png


bin_PROGRAMS =  SecretTk
COMPILATION_FLAGS =	-I@top_srcdir@/base/headers	\
		@LIBTCLPLUS_CFLAGS@ \
		-I@top_srcdir@/base/CopyrightTools \
		$(TCL_FLAGS) -DHOME=\"$(prefix)\"

SecretTk_SOURCES = SecretTk.c
SecretTk_CPPFLAGS=$(COMPILATION_FLAGS)

SecretTk_LDFLAGS= $(TCL_LDFLAGS) $(X11LIBS)


lib_LTLIBRARIES = libWait.la
libWait_la_SOURCES = WaitPackage.cpp
libWait_la_CPPFLAGS=$(COMPILATION_FLAGS)

libWait_la_LDFLAGS =	@LIBTCLPLUS_LDFLAGS@	\
			@top_builddir@/base/CopyrightTools/libLicense.la	 \
			-version-info $(SOVERSION)

libWait_la_LIBADD =  @LIBTCLPLUS_LDFLAGS@ \
	$(TCL_LDFLAGS)	$(X11LIBS)  -lLicense  \
	-lm -lstdc++ -lgcc -lc


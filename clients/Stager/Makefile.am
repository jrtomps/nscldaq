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
	UniqueInstance.tcl \
	directories.tcl \
	rsh.tcl \
	warning.tcl \
	installpkg

lib_LTLIBRARIES = libWait.la
libWait_la_SOURCES = WaitPackage.cpp

libWait_la_LDFLAGS =   -L@top_srcdir@/TCL \
	-L@top_srcdir@/Exception -L@top_srcdir@/CopyrightTools \
	-version-info 1:1:0

	  
libWait_la_LIBADD =  -lTclPlus -lException  \
	$(TCL_LDFLAGS)	$(X11LIBS)  -lLicense  \
	-lm -lstdc++ -lgcc -lc

INCLUDES = -I@top_srcdir@/Headers \
	-I@top_srcdir@/TCL \
	-I@top_srcdir@/CopyrightTools \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"
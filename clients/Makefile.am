SUBDIRS = $(VME_DRIVERDIR) \
	Headers CopyrightTools Exception PortManager TCL Security Ftp \
	VMEApi cvt SpecTclOnline DeviceSupport\
	Readout HPReadout diskrun EventLog Scaler  \
	vmetcl  \
	bufdump TclServer bcnaf TcpHoister Stager Scripts framework \
	AlarmDisplay LogDisplay ProductionReadout  HPProductionReadout \
	contrib dvdburn daqstart $(EPICSSOFTWARE) ScalerDisplay

EXTRA_DIST =  config/install-sh config/config.sub config/config.guess \
	config/ltmain.sh config/missing config/mkinstalldirs mkpackage.tcl \
	Directories.include CompilationRules.include \
	CHANGELOG daqman Makefile.am wienerdriver btdriver controlpush/Makefile.am \
	controlpush/Makefile.in

install-data-local:
	rm -f $(prefix)/Bin
	rm -f $(prefix)/Lib
	rm -f $(prefix)/Include
	rm -f $(prefix)/Etc
	$(LN_S) $(prefix)/bin $(prefix)/Bin
	$(LN_S) $(prefix)/lib $(prefix)/Lib
	$(LN_S) $(prefix)/include $(prefix)/Include
	$(LN_S) $(prefix)/etc     $(prefix)/Etc
	echo CXX=$(CXX)                 > CompilationRules.include.final
	echo CC=$(CC)                  >> CompilationRules.include.final
	echo CXXLD=$(CXX)              >> CompilationRules.include.final
	echo CCLD=$(CC)                >> CompilationRules.include.final
	cat CompilationRules.include   >> CompilationRules.include.final
	$(INSTALL_DATA)  CompilationRules.include.final $(prefix)/etc/CompilationRules.include
	echo TCLHEADERS=$(TCL_FLAGS) > Directories.include.final
	echo TCLLIBS=$(TCL_LDFLAGS)  >>Directories.include.final
	echo SPECTRODAQBINDIR=$(spectrodaq_conf_dir) >> Directories.include.final
	echo VMEHEADERS=$(VMELIBRARYHEADERS) >> Directories.include.final
	echo VMELIBRARY=$(VMELIBRARY) >> Directories.include.final
	echo X11LIBS=$(X11LIBS)       >> Directories.include.final
	cat Directories.include       >> Directories.include.final
	$(INSTALL_DATA) Directories.include.final $(prefix)/etc/Directories.include
	$(INSTALL_DATA) config.h $(prefix)/include
	$(INSTALL_PROGRAM)  daqman $(prefix)/bin
	echo "Version: " $(VERSION) build on `hostname` `date` > $(prefix)/VERSION
	mandb $(prefix)/man
	tclsh mkpackage.tcl $(prefix)/Scripts "*.tcl"
	- tclsh mkpackage.tcl $(prefix)/lib  libVme.so libWait.so 2>/dev/null




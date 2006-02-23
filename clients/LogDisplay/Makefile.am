SCRIPTFILES= LogDisplay.tcl 
PACKAGES=Hostlist.tcl
IMAGES=AddButton.gif CancelButton.gif CloseButton.gif DisButton.gif \
	ExitButton.gif FiltButton.gif HostButton.gif OkButton.gif RecovButton.gif \
	RemButton.gif UfiltButton.gif 

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/data/LogDisplay
	$(mkinstalldirs) $(prefix)/TclLibs/LogDisplay
	$(INSTALL_SCRIPT) LogDisplay $(prefix)/bin
	$(INSTALL_SCRIPT) LogDisplay.tcl $(prefix)/TclLibs/data/LogDisplay
	$(INSTALL_SCRIPT) Hostlist.tcl   $(prefix)/TclLibs/LogDisplay
	$(INSTALL_DATA)   $(IMAGES) $(prefix)/TclLibs/data/LogDisplay
	echo "pkg_mkIndex $(prefix)/TclLibs/LogDisplay *.tcl" >index.tcl
	$(TCLSH_CMD) <index.tcl


EXTRA_DIST = $(SCRIPTFILES) $(IMAGES) LogDisplay


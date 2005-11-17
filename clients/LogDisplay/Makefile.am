SCRIPTFILES=Hostlist.tcl LogDisplay.tcl 
IMAGES=AddButton.gif CancelButton.gif CloseButton.gif DisButton.gif \
	ExitButton.gif FiltButton.gif HostButton.gif OkButton.gif RecovButton.gif \
	RemButton.gif UfiltButton.gif 

install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(prefix)/Images
	$(INSTALL_SCRIPT) LogDisplay $(prefix)/bin
	$(INSTALL_DATA) $(IMAGES) $(prefix)/Images
	$(INSTALL_SCRIPT) $(SCRIPTFILES) $(prefix)/Scripts

EXTRA_DIST = $(SCRIPTFILES) $(IMAGES) LogDisplay

#EXTRA_DIST = AddButton.gif \
#	CancelButton.gif \
#	CloseButton.gif \
#	DisButton.gif \
#	ExitButton.gif \
#	FiltButton.gif \
#	HostButton.gif \
#	Hostlist.tcl \
#	LogDisplay.tcl \
#	OkButton.gif \
#	RecovButton.gif \
#	RemButton.gif \
#	UfiltButton.gif \
#	LogDisplay
#
#	mwutil.tcl \
#	tablelist.tcl \
#	tablelistSortByColumn.tcl \
#	tablelistWidget.tcl \
#	installpkg 


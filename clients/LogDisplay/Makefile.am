install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(prefix)/Images
	$(INSTALL_SCRIPT) LogDisplay $(prefix)/bin
	for f in *.gif; do $(INSTALL_DATA) $$f $(prefix)/Images; done
	for f in *.tcl; do $(INSTALL_DATA) $$f $(prefix)/Scripts; done
	./installpkg $(prefix)

EXTRA_DIST = AddButton.gif \
	CancelButton.gif \
	CloseButton.gif \
	DisButton.gif \
	ExitButton.gif \
	FiltButton.gif \
	HostButton.gif \
	Hostlist.tcl \
	LogDisplay.tcl \
	OkButton.gif \
	RecovButton.gif \
	RemButton.gif \
	UfiltButton.gif \
	mwutil.tcl \
	tablelist.tcl \
	tablelistSortByColumn.tcl \
	tablelistWidget.tcl \
	installpkg \
	LogDisplay

SCRIPT_FILES=mwutil.tcl notebook.tcl tablelist.tcl tablelistSortByColumn.tcl \
	tablelistWidget.tcl  tabnbook.tcl

install-exec-local:
	$(mkinstalldirs) $(prefix)/TclLibs/widgets
	$(INSTALL_SCRIPT) $(SCRIPT_FILES) $(prefix)/TclLibs/widgets
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/widgets *.tcl" > index.tcl
	$(TCLSH_CMD) <index.tcl

EXTRA_DIST = $(SCRIPT_FILES)
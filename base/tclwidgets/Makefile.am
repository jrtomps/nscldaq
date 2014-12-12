SCRIPT_FILES=@srcdir@/mwutil.tcl @srcdir@/notebook.tcl @srcdir@/tablelist.tcl @srcdir@/tablelistSortByColumn.tcl \
	@srcdir@/tablelistWidget.tcl  @srcdir@/tabnbook.tcl @srcdir@/FrameSequencer.tcl @srcdir@/FrameManager.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs/widgets
	$(INSTALL_SCRIPT) $(SCRIPT_FILES) @prefix@/TclLibs/widgets
	$(INSTALL_SCRIPT) @srcdir@/ReDistribNotes @prefix@/TclLibs/widgets
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/widgets *.tcl" | @TCLSH_CMD@

EXTRA_DIST = $(SCRIPT_FILES) ReDistribNotes

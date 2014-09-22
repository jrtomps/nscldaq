

DESTDIR = $(prefix)/TclLibs/OfflineEVB

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(DESTDIR)
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(DESTDIR); done
	echo "pkg_mkIndex -verbose $(DESTDIR) *.tcl" | $(TCLSH_CMD)


TCLPACKAGES=InputPipeline.tcl HoistPipeline.tcl \
            OutputPipeline.tcl \
            JobBuilderUI.tcl JobConfigUI.tcl \
            ConfigurationErrorUI.tcl \
            MissingSourceUI.tcl \
            JobProcessor.tcl RunProcessor.tcl \
            RunStatusUI.tcl  \
            FileListWidget.tcl \
            OfflineOrdererUI.tcl \
            GlobalConfigUI.tcl \
            Job.tcl

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl


EXTRA_DIST = $(TCLPACKAGES)



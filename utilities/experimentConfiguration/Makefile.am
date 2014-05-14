

PACKAGE_FILES = project.py state.py filemenu.py projectdisplay.py

MAINS = experimentConfiguration.py

INSTDIR = @prefix@/pythonLibs/nscldaq/expconfiguration

# Install hook.. create the directory  in which we'll install the modules/packages.
#                ensure there's an __init__.py there so that it's treated as a package.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	touch $(INSTDIR)/__init__.py 
	$(INSTALL_SCRIPT) $(PACKAGE_FILES) $(INSTDIR)
	$(INSTALL_SCRIPT) $(MAINS) @bindir@

check-TESTS:
	PYTHONPATH=$(INSTDIR) @PYTHON@ @srcdir@/run_all_tests.py

EXTRA_DIST = $(PACKAGE_FILES) $(MAIN_SCRIPTS) \
projectConsumerTest.py \
projectHostsTest.py \
projectLoggerTest.py \
projectOpenTest.py \
projectProgramTest.py \
project.py \
projectRingTest.py \
projectSourceTest.py \
run_all_tests.py \
schemaTest.py

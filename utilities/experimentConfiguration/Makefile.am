

PACKAGE_FILES = project.py state.py filemenu.py projectdisplay.py \
	ringspec.py

MAINS = experimentConfiguration.py

TESTS = projectConsumerTest.py \
projectHostsTest.py \
projectLoggerTest.py \
projectOpenTest.py \
projectProgramTest.py \
project.py \
projectRingTest.py \
projectSourceTest.py \
run_all_tests.py \
schemaTest.py	\
projectProducerTest.py

INSTDIR = @prefix@/pythonLibs/nscldaq/expconfiguration

# Install hook.. create the directory  in which we'll install the modules/packages.
#                ensure there's an __init__.py there so that it's treated as a package.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	touch $(INSTDIR)/__init__.py 
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@; done

check-TESTS:
	PYTHONPATH=$(INSTDIR) @PYTHON@ @srcdir@/run_all_tests.py

EXTRA_DIST = $(PACKAGE_FILES) $(MAINS) $(TESTS)


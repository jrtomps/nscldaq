PACKAGE_FILES = StateMachine.py NotReady.py

MAINS = stateManager.py

INSTDIR = @prefix@/pythonLibs/nscldaq/statemanager

##
# installation hook: create the installation directory tree.
#                    install the packges in  INSTDIR
#                    ensure there's an __init__.py which is required for
#                    package directories.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	touch $(INSTDIR)/__init__.py 
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done

EXTRA_DIST=$(PACKAGE_FILES) $(MAINS)
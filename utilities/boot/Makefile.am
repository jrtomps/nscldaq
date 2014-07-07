PYTHONPROGRAMS = boot.py
PYTHONPACKAGES = Rings.py Programs.py ssh.py config.py

PYTHONDIR      = @prefix@/pythonLibs/nscldaq/boot

install-exec-local:
	$(mkinstalldirs) @bindir@
	$(mkinstalldirs) $(PYTHONDIR)
	for f in $(PYTHONPROGRAMS); do $(INSTALL_SCRIPT)  $$f @bindir@/`basename $$f .py`; done
	$(INSTALL_SCRIPT) $(PYTHONPACKAGES) $(PYTHONDIR)
	touch $(PYTHONDIR)/__init__.py

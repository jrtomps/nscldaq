PYTHONPKGS=bootmanager.py programs.py  ssh.py

install-exec-local:
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/bootmanager
	$(INSTALL_PROGRAM) @srcdir@/*.py @prefix@/pythonLibs/nscldaq/bootmanager
	touch @prefix@/pythonLibs/nscldaq/bootmanager/__init__.py
	echo "#! @PYTHON@ "  > @bindir@/bootmanager
	echo  import nscldaq.bootmanager.bootmanager >>@bindir@/bootmanager
	chmod a+x @bindir@/bootmanager




EXTRA_DIST=bootmanager.xml $(PYTHONPKGS)  


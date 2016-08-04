PYTHONPROGS=@srcdir@/bootmanager.py 
PYTHONPKGS=@srcdir@/programs.py  @srcdir@/ssh.py @srcdir@/eventbuilders.py @srcdir@/rings.py

install-exec-local:
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/bootmanager
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/programs
	$(INSTALL_PROGRAM) $(PYTHONPROGS) @prefix@/pythonLibs/nscldaq/bootmanager
	touch @prefix@/pythonLibs/nscldaq/bootmanager/__init__.py
	$(INSTALL_PROGRAM) $(PYTHONPKGS) @prefix@/pythonLibs/nscldaq/programs
	touch @prefix@/pythonLibs/nscldaq/programs/__init__.py
	echo "#! @PYTHON@ "  > @bindir@/bootmanager
	echo  import nscldaq.bootmanager.bootmanager >>@bindir@/bootmanager
	chmod a+x @bindir@/bootmanager
	$(INSTALL_PROGRAM) @srcdir@/boot-service.sh  @bindir@/boot-service




EXTRA_DIST=bootmanager.xml $(PYTHONPKGS) $(PYTHONPROGS)  boot-service.sh


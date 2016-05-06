PKGDIR=@prefix@/pythonLibs/nscldaq/vardb
PYPROGRAMS = addrdo.in rdoonoff.in lsrdo.in rmrdo.in resetrdo.in rdosalone.in

install-exec-hook:
	$(mkinstalldirs) @bindir@
	$(mkinstalldirs) $(PKGDIR)
	$(INSTALL_SCRIPT) @srcdir@/varmgrcommon.py $(PKGDIR)
	$(INSTALL_SCRIPT) @srcdir@/MakeRunControl.tcl @bindir@
	for f in $(PYPROGRAMS); do $(INSTALL_SCRIPT) `basename $$f .in` @bindir@; done



EXTRA_DIST=MakeRunControl.tcl $(PYPROGRAMS) varmgrcommon.py rdotools.xml


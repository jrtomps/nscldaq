install-exec-local:
	$(mkinstalldirs) $(prefix)/contrib          $(prefix)/contrib/lecroy
	$(INSTALL_PROGRAM) $(srcdir)/LeCroy1440.tcl $(prefix)/contrib/lecroy
	$(INSTALL_DATA)    $(srcdir)/nscl.gif       $(prefix)/contrib/lecroy

EXTRA_DIST = LeCroy1440.tcl nscl.gif
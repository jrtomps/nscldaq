pwdinstall-exec-local:
	$(mkinstalldirs) $(prefix)/contrib $(prefix)/contrib/caenv812
	$(INSTALL_SCRIPT) *.tcl            $(prefix)/contrib/caenv812
	$(INSTALL_DATA)   *.cfd            $(prefix)/contrib/caenv812
	$(INSTALL_DATA)   README           $(prefix)/contrib/caenv812
	./installpkg $(prefix)/contrib/caenv812

EXTRA_DIST = CFD812.tcl \
	CFDState.tcl    \
	README          \
	Setup.cfd       \
	Slot2.cfd       \
	Slot4.cfd       \
	cfd.tcl         \
	failsafe.cfd    \
	loadcfd.tcl     \
	mysetup.cfd     \
	testing.cfd    \
	Makefile.am installpkg

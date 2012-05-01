install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(INSTALL_DATA) VMUSBDriverIncludes drivertemplate.cpp @prefix@/vmusbdriver
	$(INSTALL_DATA) Makefile-template @prefix@/vmusbdriver/Makefile

EXTRA_DIST=drivertemplate.cpp Makefile-template.in